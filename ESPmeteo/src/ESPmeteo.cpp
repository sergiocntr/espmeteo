//#define DEBUGMIO
#include "ESPmeteo.h"
void preinit() {
  // Global WiFi constructors are not called yet
  // (global class instances like WiFi, Serial... are not yet initialized)..
  // No global object methods or C++ exceptions can be called in here!
  //The below is a static class method, which is similar to a function, so it's ok.
  ESP8266WiFiClass::preinitWiFiOff();
}
void user_init() {
  // Global WiFi constructors are not called yet
  // (global class instances like WiFi, Serial... are not yet initialized)..
  // No global object methods or C++ exceptions can be called in here!
  //The below is a static class method, which is similar to a function, so it's ok.
  ESP8266WiFiClass::preinitWiFiOff();
}
void reinit(uint8_t value){
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
	//DEBUG_PRINT("WIFI OK");
	if(reconnect()){
		sendThing();
		//client.disconnect();
		//smartDelay(100);
	}
	wifi_initiate = millis();
  while (!c.connect("www.google.com", 80 )) {
    if (millis() - wifi_initiate > 5000L) {
      //DEBUG_PRINT("conn internet FAIL! ");
			storeData(value);
      shutDownNow();
    }
    delay(1000);
  }
  char risp =1;
	for (char i = 0; i < 4; i++) {
		risp = printWEBJSON(value);
		if(risp==0) {
			writeEEPROM(nValuesAddr,0); //reset storage records nr on I2C eeprom
			break;
		}
  }
}
void setup(){
	wifi_set_sleep_type(LIGHT_SLEEP_T);
	WiFi.mode(WIFI_OFF);
	WiFi.forceSleepBegin();
	delay(100);
	#ifdef DEBUGMIO
		Serial.begin(9600);
  	delay(5000);
	#endif
	//DEBUG_PRINT("Booting!");
	Wire.begin(default_sda_pin, default_scl_pin);
	delay(10);
	uint8_t value = readEEPROM(nValuesAddr);
	if(value==255) {
		value = 0;
		writeEEPROM(nValuesAddr,value); //update storage records nr on I2C eeprom
	}
	delay(10);
	char risp = requestSensorsValues();
	delay(10);
	switch (risp) {
		case 0:
			//DEBUG_PRINT("Dati Recuperati OK!!");
			break;
		case 1:
			//DEBUG_PRINT("Sonda Guasta!!");
			shutDownNow();
			break;
	}
	//#ifdef DEBUGMIO
		//DEBUG_PRINT("Press: " + String(retmet.externalPressure));
		//DEBUG_PRINT("Temp: " + String(retmet.temperatureBMP));
		//DEBUG_PRINT("Hum: " + String(retmet.humidityBMP));
		//DEBUG_PRINT("Voltage: " + String(retmet.battery));
	//delay(100);
	//#endif
  String clientId = String(marinerId);
  clientId += String(random(0xffff), HEX);
  yield();
  setIP(marinerUan,clientId.c_str());
  uint8_t myvalue=connectWiFi();
  if(myvalue!=0){
    storeData(value);
    shutDownNow();
  }
  //delay(10);
  client.setServer(mqtt_server, mqtt_port);
  //client.setCallback(callback);
  delay(10);
  //checkmio = connectMQTT();
  myvalue=connectMQTT();
  if(myvalue==0)
  {
    client.publish(logTopic, "marinerUan connesso");
    client.loop();
    sendThing();
    delay(10);
  }
  //reinit(value);
  delay(50);
  risp =1;
	for (char i = 0; i < 4; i++) {
		risp = printWEBJSON(value);
		if(risp==0) {
			writeEEPROM(nValuesAddr,0); //reset storage records nr on I2C eeprom
			break;
		}
  }
  //delay(50);
  //checkForUpdates(versione);
}
void callback(char* topic, byte* payload, unsigned int length){}
bool reconnect() {
  if (client.connected()) return true;
  for (char i = 0; i < 3; i++)
  {
    //DEBUG_PRINT("Attempting MQTT connection...");
    if (client.connect("marinerUan",mqttUser,mqttPass))
    {
      //DEBUG_PRINT("connected");
      client.publish(logTopic, "marinerUan connesso");
      client.loop();
			delay(10);
      return true;
    }
    else
    {
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

	//DEBUG_PRINT("spegniti");
	Wire.begin(default_sda_pin, default_scl_pin);
	//delay(50);
	Wire.beginTransmission (2);
  Wire.write (20);
  Wire.endTransmission(true);
}
void loop(){
  WiFi.disconnect(true);
  delay(10);
	WiFi.mode( WIFI_OFF );
	WiFi.forceSleepBegin();
	delay( 10 );
	shutDownNow();
	delay(1000);
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
  //if(voltage > 5000){
    //setup();

  //}
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
    yield();
		for (int i = nrRecords ; i > 0 ; i--){
			readStructEEPROM(32 * i); // read I2C eeprom
			delay(10);
			jsonHum.add(met.humidityBMP);
		  jsonTemp.add(met.temperatureBMP);
		  jsonPress.add(met.externalPressure);
		  jsonBat.add(met.battery);
		}

		//DEBUG_PRINT("Buffer multi: " + String(JSONbuffer.size()));
	smartDelay(100);
	int httpResponseCode=0;
	String s="";
	JSONencoder.prettyPrintTo(s);
	yield();
	http.begin(c,post_serverJSON);
	httpResponseCode = http.PUT(s);
	//DEBUG_PRINT(s);
	smartDelay(100);
	http.end();  //Free resources
	if(httpResponseCode==200){
		String response = http.getString();                       //Get the response to the request
		//DEBUG_PRINT(httpResponseCode);   //Print return code
	  //DEBUG_PRINT(response);           //Print request answer
		return 0;
 	}else{
		//DEBUG_PRINT("Error on sending POST: ");
	  //DEBUG_PRINT(httpResponseCode);
		return 1;
	}

 	///return true;
}
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
	check=send(extSensTopic, JSONmessageBuffer);
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
