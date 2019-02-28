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

	//READ SENSORS
	requestSensorsValues();
	#ifdef DEBUGMIO
		//DEBUG__PRINT("there are " + String(value) + " stored values to send...");
		//DEBUG__PRINT("Press: " + String(retmet.externalPressure));
		//DEBUG__PRINT("Temp: " + String(retmet.temperatureBMP));
		//DEBUG__PRINT("Hum: " + String(retmet.humidityBMP));
	#endif
	//CHECK INTERNET CONNECTION
	WiFi.forceSleepWake();
  delay(100);
	WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.config(marinerUan, gateway, subnet,dns1); // Set static IP (2,7s) or 8.6s with DHCP  + 2s on battery
	client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
	for (char i = 0; i < 10; i++) if (WiFi.status() != WL_CONNECTED) delay(500); else break;
	if(WiFi.status() != WL_CONNECTED){
		WiFi.disconnect(true);
    WiFi.mode( WIFI_OFF );
    WiFi.forceSleepBegin();
    delay( 10 );
		storeData(value);
		shutDownNow();
	}else{
		//DEBUG__PRINT("WIFI OK");
		reconnect();
		sendThing();
		client.disconnect();
		client.loop();
		smartDelay(100);
		unsigned long wifi_initiate = millis();
	  while (!c.connect("www.google.com", 80 )) {
	    if (millis() - wifi_initiate > 15000L) {
	      //DEBUG__PRINT("conn internet FAIL! ");
	      shutDownNow();
	    }
	    smartDelay(2000);
	  }
		printWEBJSON(value);
		writeEEPROM(nValuesAddr,0); //reset storage records nr on I2C eeprom
	}
}
void callback(char* topic, byte* payload, unsigned int length)
{}
void reconnect() {
  if ((WiFi.status() == WL_CONNECTED) && (!client.connected()))
  {
    for (char i = 0; i < 10; i++)
    {
      //DEBUG__PRINT("Attempting MQTT connection...");
      if (client.connect("marinerUan",mqttUser,mqttPass))
      {
        //DEBUG__PRINT("connected");
        client.publish(logTopic, "marinerUan connesso");
        client.loop();
        break;
      }
      else
      {
        //DEBUG__PRINT("failed, rc=");
        //DEBUG__PRINT(client.state());
        //DEBUG__PRINT(" try again in 5 seconds");
        smartDelay(500);
      }
    }
  }
}
void storeData(uint8_t nrRecords){
	uint16_t availAddress = 32 * nrRecords;	//MeteoData is 24 bytes long so..
	//compile struct object with current data
	met.battery = voltage;
	met.humidityBMP = humidityBMP;
	met.temperatureBMP = temperatureBMP;
	met.externalPressure = p0;
	writeStructEEPROM(availAddress);	//write struct on I2C eeprom
	nrRecords++; //add record's nr
	writeEEPROM(nValuesAddr,nrRecords); //update storage records nr on I2C eeprom
}
void shutDownNow(){
	//this tell to attiny to power down ESP
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
			//DEBUG__PRINT("mando dati registrati record " + String(i));
			delay(10);
			jsonHum.add(met.humidityBMP);
		  jsonTemp.add(met.temperatureBMP);
		  jsonPress.add(met.externalPressure);
		  jsonBat.add(met.battery);
		}
		#ifdef DEBUGMIO
		//DEBUG__PRINT("Buffer multi: " + String(JSONbuffer.size()));
		DEBUG_MQTT("preparato dati memorizzati");
		#endif
		smartDelay(10);
	int httpResponseCode=0;
	//uint8_t check=0;
	String s="";
	JSONencoder.prettyPrintTo(s);
	smartDelay(10);
	http.begin(post_serverJSON);
	httpResponseCode = http.PUT(s);
	#ifdef DEBUGMIO
	//DEBUG__PRINT(s);
	#endif
	if(httpResponseCode>0){
		String response = http.getString();                       //Get the response to the request
		//DEBUG__PRINT(httpResponseCode);   //Print return code
	  //DEBUG__PRINT(response);           //Print request answer

 	}else{
		//DEBUG__PRINT("Error on sending POST: ");
	  //DEBUG__PRINT(httpResponseCode);
	}
 	http.end();  //Free resources
 	return true;
}

//MQTT//////////////////////////////////////////////////////////////
void sendThing(){
	StaticJsonBuffer<300> JSONbuffer;
	JsonObject& JSONencoder = JSONbuffer.createObject();
  JSONencoder["topic"] = "Terrazza";
	JSONencoder["Hum"] =retmet.humidityBMP;
  JSONencoder["Temp"] =retmet.temperatureBMP;
  JSONencoder["Press"] =retmet.externalPressure;

  char JSONmessageBuffer[100];
	smartDelay(10);
	//int check =0;
	JSONencoder.printTo(JSONmessageBuffer);
	if(client.connected()) {
		client.publish(extSensTopic, JSONmessageBuffer,true);
		smartDelay(10);
		#ifdef DEBUGMIO
		//DEBUG__PRINT("Size JSONBuffer: " + String(JSONbuffer.size()));
		//DEBUG__PRINT("mandato dati a mqtt: " + String(check));
		//DEBUG__PRINT(String(JSONmessageBuffer));
		#endif
	}else {
		//DEBUG__PRINT("** ERROR MQTT NOT CONNECTED");
		return;
	}

	client.disconnect();
	client.loop();
}
void smartDelay(unsigned long ms){
  unsigned long start = millis();
  do
  {
		client.loop();
    yield();

  } while (millis() - start < ms);
}
