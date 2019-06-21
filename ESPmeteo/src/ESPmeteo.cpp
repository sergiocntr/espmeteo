#include "ESPmeteo.h"
void setup(){
	wifi_set_sleep_type(LIGHT_SLEEP_T);
	WiFi.mode(WIFI_OFF);
	WiFi.forceSleepBegin();
	delay(100);
	#ifdef DEBUGMIO
		Serial.begin(9600);
  	delay(5000);
		Serial.println("Booting!");
	#endif

	Wire.begin(default_sda_pin, default_scl_pin);
	uint8_t nrRecord = readEEPROM(nValuesAddr);
	if(nrRecord==255) {
		nrRecord = 0;
		writeEEPROM(nValuesAddr,nrRecord); //update storage records nr on I2C eeprom
	}
	Serial.println("Stored: " + String(nrRecord));
	delay(10);
	uint8_t risp = requestSensorsValues();
	Serial.println("BMP: " + String(risp));
	//delay(1000);
	yield();
	if(risp==1) shutDownNow();
	//#ifdef DEBUGMIO
		//Serial.println("Pressp=: " + String(p0));
		Serial.println("Press: " + String(met.externalPressure));
		Serial.println("Temp: " + String(met.temperatureBMP));
		Serial.println("Hum: " + String(met.humidityBMP));
		Serial.println("Voltage: " + String(met.battery));

	//#endif
	yield();
	setIP(marinerUan,marinerId);
	//delay(100);
  risp=connectWiFi();
  Serial.println("WIFI: " + String(risp));
  if(risp!=0){
    storeData(nrRecord);
    shutDownNow();
  }
	client.setServer(mqtt_server, mqtt_port);
	//client.setCallback(callback);
	delay(10);
	//int8_t checkMQTT = connectMQTT();
	//if (!client.connected()){
	  for (char i = 0; i < 3; i++)
	  {
	    //DEBUG_PRINT("Attempting MQTT connection...");
	    if (client.connect("marinerUan",mqttUser,mqttPass))
	    {
	      Serial.println("connected");
	      //client.publish(logTopic, "marinerUan connesso");
	      client.loop();
				client.publish(logTopic, "marinerUan connesso");
				Serial.println("publish");
		    client.loop();
		    sendThing();
		    delay(10);
				//delay(10);
	      //return true;
	    }
	    else
	    {
				Serial.println("...");
	      smartDelay(500);
	    }
	  }
	//}
	yield();
	risp =1;
	for (uint8_t i = 0; i < 4; i++) {
		risp = printWEBJSON(nrRecord);
    Serial.println("PRINT WEB: " + String(risp));
		if(risp==0) {
			writeEEPROM(nValuesAddr,0); //reset storage records nr on I2C eeprom
			break;
		}
  }

  smartDelay(50);
}
void loop(){
	if (client.connected()) client.disconnect();
	smartDelay(50);
  WiFi.disconnect(true);
  //delay(10);
	//WiFi.mode( WIFI_OFF );
	//WiFi.forceSleepBegin();
	delay( 10 );
	shutDownNow();
	delay(1000);
}
void callback(char* topic, byte* payload, unsigned int length){}
void shutDownNow(){
	//this tell to attiny to power down ESP

	Serial.println("spegniti");
	Wire.begin(default_sda_pin, default_scl_pin);
	//delay(50);
	Wire.beginTransmission (2);
  Wire.write (20);
  Wire.endTransmission(true);
}


void storeData(uint8_t nrRecords){
	uint16_t availAddress = 32 * nrRecords;	//MeteoData is 24 bytes ,we use 32 bytes page write
	writeStructEEPROM(availAddress);	//write struct on I2C eeprom
	nrRecords++; //add record's nr
	writeEEPROM(nValuesAddr,nrRecords); //update storage records nr on I2C eeprom
}
uint8_t requestSensorsValues(){
	Wire.requestFrom(SLAVE_ATTINY_ADDRESS, 2);    // request 2 bytes from slave device #2---- Wire.requestFrom (SLAVE_ADDRESS, responseSize);
	uint8_t i=0;
	while (Wire.available()){
		dati[i] = Wire.read();    // receive a byte as character
		i++;
  }
	voltage = (dati[1]<<8) | dati[0];
	met.battery=voltage;
  uint8_t res= bm(met);									// read BMP080 values
	res= bm(met);									// read BMP080 values
	return res;
}
uint8_t printWEBJSON(uint8_t nrRecords) {//timeAvailable -> live mesaures
	StaticJsonBuffer<2000> JSONbuffer;
	JsonObject& JSONencoder = JSONbuffer.createObject();
	JsonArray& jsonHum = JSONencoder.createNestedArray("hum");
  JsonArray& jsonTemp = JSONencoder.createNestedArray("temp");
  JsonArray& jsonPress = JSONencoder.createNestedArray("press");
  JsonArray& jsonBat = JSONencoder.createNestedArray("bat");
  jsonHum.add(met.humidityBMP);
  jsonTemp.add(met.temperatureBMP);
  jsonPress.add(met.externalPressure);
  jsonBat.add(met.battery);
  yield();
	if(nrRecords>0){
	  nrRecords--;
		Serial.println(nrRecords);
		for (int i =  nrRecords ; i >= 0 ; i--){
			Serial.println("non devi leggerlo");
			readStructEEPROM(32 * i); // read I2C eeprom
			delay(10);
			jsonHum.add(met.humidityBMP);
		  jsonTemp.add(met.temperatureBMP);
		  jsonPress.add(met.externalPressure);
		  jsonBat.add(met.battery);
		}
	}
  smartDelay(100);
	int httpResponseCode=0;
	String s="";
	JSONencoder.prettyPrintTo(s);
	yield();
	HTTPClient http;
  http.begin(c,post_serverJSON);
	httpResponseCode = http.PUT(s);
	Serial.println(s);
	smartDelay(1500);
	http.end();  //Free resources
	if(httpResponseCode==200){
		//String response = http.getString();                       //Get the response to the request
		Serial.println(httpResponseCode);   //Print return code
	  //Serial.println(response);           //Print request answer
		return 0;
 	}else{
		Serial.println("Error on sending POST: ");
	  Serial.println(httpResponseCode);
		return 1;
	}

 	///return true;
}
bool sendThing(){
	StaticJsonBuffer<300> JSONbuffer;
	JsonObject& JSONencoder = JSONbuffer.createObject();
  JSONencoder["topic"] = "Terrazza";
	JSONencoder["Hum"] =met.humidityBMP;
  JSONencoder["Temp"] =met.temperatureBMP;
  JSONencoder["Press"] =met.externalPressure;
  char JSONmessageBuffer[100];
	smartDelay(10);
	bool check =0;
	JSONencoder.printTo(JSONmessageBuffer);
	yield();
	check=client.publish(extSensTopic,JSONmessageBuffer);
	Serial.println("Publish: " + String(check));
	//check=send(extSensTopic, JSONmessageBuffer);
	client.loop();
	smartDelay(100);
	client.disconnect();
	client.loop();
	return check;
}
void smartDelay(unsigned long ms){
  unsigned long start = millis();
  do
  {
		client.loop();
    delay(10);

  } while (millis() - start < ms);
}
