//#define DEBUGMIO
#include "ESPmeteo.h"
void setup(){
	wifi_set_sleep_type(LIGHT_SLEEP_T);
	WiFi.mode(WIFI_OFF);
	WiFi.forceSleepBegin();
	delay(100);
	#ifdef DEBUGMIO
		Serial.begin(9600);
  	delay(5000);
	#endif
	DEBUG_PRINT("Booting!");
	Wire.begin(default_sda_pin, default_scl_pin);
	delay(10);
	uint8_t value = readEEPROM(nValuesAddr);
	if(value==255) {
		value = 0;
		writeEEPROM(nValuesAddr,value); //update storage records nr on I2C eeprom
	}
	requestSensorsValues();
	DEBUG_PRINT("there are " + String(value) + " stored values to send...");
	DEBUG_PRINT("Press: " + String(retmet.externalPressure));
	DEBUG_PRINT("Temp: " + String(retmet.temperatureBMP));
	DEBUG_PRINT("Hum: " + String(retmet.humidityBMP));

	//CHECK INTERNET CONNECTION
	WiFi.forceSleepWake();
  delay(100);
	WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.config(marinerUan, gateway, subnet,dns1); // Set static IP (2,7s) or 8.6s with DHCP  + 2s on battery
	client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
	unsigned long wifi_initiate = millis();
  while (WiFi.status() != WL_CONNECTED){
		delay(500);
		if (millis() - wifi_initiate > 5000L){
			storeData(value);
			shutDownNow();
		}
	}
	DEBUG_PRINT("WIFI OK");
	if(reconnect()){
		sendThing();
		client.disconnect();
		smartDelay(100);
	}
	wifi_initiate = millis();
  while (!c.connect("www.google.com", 80 )) {
    if (millis() - wifi_initiate > 5000L) {
      DEBUG_PRINT("conn internet FAIL! ");
			storeData(value);
      shutDownNow();
    }
    smartDelay(1000);
  }
	for (char i = 0; i < 4; i++) {
		bool check = printWEBJSON(value);
		if(check) {
			writeEEPROM(nValuesAddr,0); //reset storage records nr on I2C eeprom
			shutDownNow();
		}else storeData(value);
	}
}
void callback(char* topic, byte* payload, unsigned int length)
{}
bool reconnect() {
  if (client.connected()) return true;
  for (char i = 0; i < 5; i++)
  {
    DEBUG_PRINT("Attempting MQTT connection...");
    if (client.connect("marinerUan",mqttUser,mqttPass))
    {
      DEBUG_PRINT("connected");
      client.publish(logTopic, "marinerUan connesso");
      client.loop();
			delay(10);
      return true;
    }
    else
    {
      DEBUG_PRINT("failed, rc=");
      DEBUG_PRINT(client.state());
      DEBUG_PRINT(" try again in 5 seconds");
      smartDelay(500);
    }
  }
	return false;
}
void storeData(uint8_t nrRecords){
	uint16_t availAddress = 32 * nrRecords;	//MeteoData is 24 bytes long so..
	//compile struct object with current data
	//met.battery = voltage;
	//met.humidityBMP = humidityBMP;
	//met.temperatureBMP = temperatureBMP;
	//met.externalPressure = p0;
	writeStructEEPROM(availAddress);	//write struct on I2C eeprom
	nrRecords++; //add record's nr
	writeEEPROM(nValuesAddr,nrRecords); //update storage records nr on I2C eeprom
}
void shutDownNow(){
	//this tell to attiny to power down ESP
	WiFi.disconnect(true);
	WiFi.mode( WIFI_OFF );
	WiFi.forceSleepBegin();
	delay( 10 );
	DEBUG_PRINT("spegniti");
	delay(50);
	Wire.begin(default_sda_pin, default_scl_pin);
	//delay(50);
	Wire.beginTransmission (2);
  Wire.write (20);
  Wire.endTransmission(true);
}
void loop(){
	shutDownNow();
	delay(3000);
}
void requestSensorsValues(){
	for (int i=0; i <= 2; i++){
    Wire.requestFrom(2, 2);    // request 2 bytes from slave device #2---- Wire.requestFrom (SLAVE_ADDRESS, responseSize);
    dati[i] = Wire.read();    // receive a byte as character
  }
  voltage = (dati[1]<<8) | dati[0];
  bm(temperatureBMP,humidityBMP,p0);									// read BMP080 values
	retmet.battery = voltage;
	retmet.humidityBMP = humidityBMP;
	retmet.temperatureBMP = temperatureBMP;
	retmet.externalPressure = p0;
}
bool printWEBJSON(uint8_t nrRecords) {//timeAvailable -> live mesaures
	StaticJsonBuffer<2000> JSONbuffer;
	JsonObject& JSONencoder = JSONbuffer.createObject();
	//if(nrRecords>0)
	//{
		JsonArray& jsonHum = JSONencoder.createNestedArray("hum");
	  JsonArray& jsonTemp = JSONencoder.createNestedArray("temp");
	  JsonArray& jsonPress = JSONencoder.createNestedArray("press");
	  JsonArray& jsonBat = JSONencoder.createNestedArray("bat");
		jsonHum.add(retmet.humidityBMP);
		jsonTemp.add(retmet.temperatureBMP);
		jsonPress.add(retmet.externalPressure);
		jsonBat.add(retmet.battery);
		for (int i = nrRecords ; i > 0 ; i--){
			readStructEEPROM(32 * i); // read I2C eeprom
			DEBUG_PRINT("mando dati registrati record " + String(i));
			delay(10);
			jsonHum.add(met.humidityBMP);
		  jsonTemp.add(met.temperatureBMP);
		  jsonPress.add(met.externalPressure);
		  jsonBat.add(met.battery);
		}

		DEBUG_PRINT("Buffer multi: " + String(JSONbuffer.size()));
		smartDelay(100);
	int httpResponseCode=0;
	String s="";
	JSONencoder.prettyPrintTo(s);
	smartDelay(100);
	http.begin(post_serverJSON);
	httpResponseCode = http.PUT(s);
	DEBUG_PRINT(s);
	smartDelay(100);
	http.end();  //Free resources
	if(httpResponseCode==200){
		String response = http.getString();                       //Get the response to the request
		DEBUG_PRINT(httpResponseCode);   //Print return code
	  DEBUG_PRINT(response);           //Print request answer
		return true;
 	}else{
		DEBUG_PRINT("Error on sending POST: ");
	  DEBUG_PRINT(httpResponseCode);
		return false;
	}

 	///return true;
}

//MQTT//////////////////////////////////////////////////////////////
bool sendThing(){
	StaticJsonBuffer<300> JSONbuffer;
	JsonObject& JSONencoder = JSONbuffer.createObject();
  JSONencoder["topic"] = "Terrazza";
	JSONencoder["Hum"] =retmet.humidityBMP;
  JSONencoder["Temp"] =retmet.temperatureBMP;
  JSONencoder["Press"] =retmet.externalPressure;

  char JSONmessageBuffer[100];
	smartDelay(10);
	bool check =0;
	JSONencoder.printTo(JSONmessageBuffer);
	if(client.connected()) {
		check=client.publish(extSensTopic, JSONmessageBuffer,true);
		smartDelay(10);
		#ifdef DEBUGMIO
		DEBUG_PRINT("Size JSONBuffer: " + String(JSONbuffer.size()));
	DEBUG_PRINT("mandato dati a mqtt: " + String(check));
		DEBUG_PRINT(String(JSONmessageBuffer));
		#endif
	}else {
		DEBUG_PRINT("** ERROR MQTT NOT CONNECTED");
		//return;
	}

	client.disconnect();
	client.loop();
	if(check) return true;
	return false;
}
void smartDelay(unsigned long ms){
  unsigned long start = millis();
  do
  {
		client.loop();
    yield();

  } while (millis() - start < ms);
}
