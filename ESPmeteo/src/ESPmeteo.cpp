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
	//DEBUG__PRINT("Booting!");
	Wire.begin(default_sda_pin, default_scl_pin);
	delay(10);
	uint8_t value = readEEPROM(nValuesAddr);
	if(value==255) {
		value = 0;
		writeEEPROM(nValuesAddr,value); //update storage records nr on I2C eeprom
	}
	//DEBUG__PRINT("There are " + String(value) + " stored values to send...");
	delay(10);
	char risp = requestSensorsValues();
	delay(10);
	switch (risp) {
		case 0:
			//DEBUG__PRINT("Dati Recuperati OK!!");
			break;
		case 1:
			//DEBUG__PRINT("Sonda Guasta!!");
			shutDownNow();
			break;
	}
	#ifdef DEBUGMIO
		//DEBUG__PRINT("Press: " + String(retmet.externalPressure));
		//DEBUG__PRINT("Temp: " + String(retmet.temperatureBMP));
		//DEBUG__PRINT("Hum: " + String(retmet.humidityBMP));
		//DEBUG__PRINT("Voltage: " + String(retmet.battery));
	delay(100);
	#endif
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
	//DEBUG__PRINT("WIFI OK");
	if(reconnect()){
		sendThing();
		client.disconnect();
		smartDelay(100);
	}
	wifi_initiate = millis();
  while (!c.connect("www.google.com", 80 )) {
    if (millis() - wifi_initiate > 5000L) {
      //DEBUG__PRINT("conn internet FAIL! ");
			storeData(value);
      shutDownNow();
    }
    smartDelay(1000);
  }
	risp=1;
	for (char i = 0; i < 4; i++) {
		risp= printWEBJSON(value);
		if(risp==0) {
			writeEEPROM(nValuesAddr,0); //reset storage records nr on I2C eeprom
			break;
		}else smartDelay(1000);
	}
	if(risp==1) storeData(value);
}
void callback(char* topic, byte* payload, unsigned int length)
{}
bool reconnect() {
  if (client.connected()) return true;
  for (char i = 0; i < 2; i++)
  {
    //DEBUG__PRINT("Attempting MQTT connection...");
    if (client.connect("marinerUan",mqttUser,mqttPass))
    {
      //DEBUG__PRINT("connected");
      client.publish(logTopic, "marinerUan connesso");
      client.loop();
			delay(10);
      return true;
    }
    else
    {
      //DEBUG__PRINT("failed, rc=");
      //DEBUG__PRINT(client.state());
      //DEBUG__PRINT(" try again in 5 seconds");
      smartDelay(500);
    }
  }
	return false;
}
void storeData(uint8_t nrRecords){
	uint16_t availAddress = 32 * nrRecords;	//MeteoData is 24 bytes long so..
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
	//DEBUG__PRINT("spegniti");
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
char requestSensorsValues(){
	for (int i=0; i <= 2; i++){
    Wire.requestFrom(2, 2);    // request 2 bytes from slave device #2---- Wire.requestFrom (SLAVE_ADDRESS, responseSize);
    dati[i] = Wire.read();    // receive a byte as character
  }
  voltage = (dati[1]<<8) | dati[0];
  char res= bm(temperatureBMP,humidityBMP,p0);									// read BMP080 values
	retmet.battery = voltage;
	retmet.humidityBMP = humidityBMP;
	retmet.temperatureBMP = temperatureBMP;
	retmet.externalPressure = p0;
	return res;
}
char printWEBJSON(uint8_t nrRecords) {//timeAvailable -> live mesaures
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
			//DEBUG__PRINT("mando dati registrati record " + String(i));
			delay(10);
			jsonHum.add(met.humidityBMP);
		  jsonTemp.add(met.temperatureBMP);
		  jsonPress.add(met.externalPressure);
		  jsonBat.add(met.battery);
		}

		//DEBUG__PRINT("Buffer multi: " + String(JSONbuffer.size()));
		smartDelay(100);
	int httpResponseCode=0;
	String s="";
	JSONencoder.prettyPrintTo(s);
	smartDelay(100);
	http.begin(post_serverJSON);
	httpResponseCode = http.PUT(s);
	//DEBUG__PRINT(s);
	smartDelay(100);
	http.end();  //Free resources
	if(httpResponseCode==200){
		String response = http.getString();                       //Get the response to the request
		//DEBUG__PRINT(httpResponseCode);   //Print return code
	  //DEBUG__PRINT(response);           //Print request answer
		return 0;
 	}else{
		//DEBUG__PRINT("Error on sending POST: ");
	  //DEBUG__PRINT(httpResponseCode);
		return 1;
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
		//DEBUG__PRINT("Size JSONBuffer: " + String(JSONbuffer.size()));
	//DEBUG__PRINT("mandato dati a mqtt: " + String(check));
		//DEBUG__PRINT(String(JSONmessageBuffer));
		#endif
	}else {
		//DEBUG__PRINT("** ERROR MQTT NOT CONNECTED");
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
