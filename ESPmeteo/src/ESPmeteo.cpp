#include "ESPmeteo.h"
void setup(){
	wifi_set_sleep_type(LIGHT_SLEEP_T);
	WiFi.mode(WIFI_OFF);
	WiFi.forceSleepBegin();
	delay(100);
	#ifdef DEBUGMIO
		Serial.begin(9600);
  	delay(5000);
		#define DEBPRINT(str) Serial.println(str);
		DEBPRINT("Booting!");
	#else
		#define DEBPRINT(str)
	#endif

	Wire.begin(default_sda_pin, default_scl_pin);
	uint8_t nrRecord = readEEPROM(nValuesAddr);
	if(nrRecord==255) {
		nrRecord = 0;
		writeEEPROM(nValuesAddr,nrRecord); //update storage records nr on I2C eeprom
	}
	
	delay(10);
	uint8_t risp = requestSensorsValues();
	if(risp==1) shutDownNow();
	#ifdef DEBUGMIO
		DEBPRINT("Stored: " + String(nrRecord));
		DEBPRINT("BMP: " + String(risp));
		DEBPRINT("Press: " + String(met.externalPressure));
		DEBPRINT("Temp: " + String(met.temperatureBMP));
		DEBPRINT("Hum: " + String(met.humidityBMP));
		DEBPRINT("Voltage: " + String(met.battery));
	#endif
	yield();
	setIP(marinerUan,marinerId);
	risp=connectWiFi();
  DEBPRINT("WIFI: " + String(risp));
  if(risp!=0){
    storeData(nrRecord);
    shutDownNow();
  }
	client.setServer(mqtt_server, mqtt_port);
	delay(10);
	for (char i = 0; i < 3; i++)
	  {
	    DEBPRINT("Attempting MQTT connection...");
	    if (client.connect("marinerUan",mqttUser,mqttPass))
	    {
	      DEBPRINT("connected");
	      client.loop();
				client.publish(logTopic, "marinerUan connesso");
				DEBPRINT("publish");
		    client.loop();
		    sendThing();
		    delay(10);
				break;
	    }
	    else
	    {
				DEBPRINT("...");
	      smartDelay(500);
	    }
	  }
	yield();
	risp =1;
	for (uint8_t i = 0; i < 4; i++) {
		risp = printWEBJSON(nrRecord);
    DEBPRINT("PRINT WEB: " + String(risp));
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
  delay( 10 );
	shutDownNow();
	delay(1000);
}
void callback(char* topic, byte* payload, unsigned int length){}
void shutDownNow(){
	//this tell to attiny to power down ESP

	DEBPRINT("spegniti");
	Wire.begin(default_sda_pin, default_scl_pin);
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
		DEBPRINT(nrRecords);
		for (int i =  nrRecords ; i >= 0 ; i--){
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
	DEBPRINT(s);
	smartDelay(1500);
	http.end();  //Free resources
	if(httpResponseCode==200){
		DEBPRINT(httpResponseCode);   //Print return code
	  //DEBPRINT(response);           //Print request answer
		return 0;
 	}else{
		DEBPRINT("Error on sending POST: ");
	  DEBPRINT(httpResponseCode);
		return 1;
	}
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
	DEBPRINT("Publish: " + String(check));
	smartDelay(100);
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
