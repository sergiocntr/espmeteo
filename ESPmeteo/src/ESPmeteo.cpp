#include "ESPmeteo.h"
void blinkLed(uint8_t volte,bool l){
uint16_t tempo;
if(l){tempo=250;}else{tempo=500;}
  for (uint8_t i = 0; i < volte; i++)
  {
    digitalWrite(LED_BUILTIN,LOW);
    delay(tempo);
    digitalWrite(LED_BUILTIN,HIGH);
    delay(tempo);
  }
}
void setWifiOff(){
	wifi_set_sleep_type(LIGHT_SLEEP_T);
	WiFi.persistent(false);   // Solve possible wifi init errors (re-add at 6.2.1.16 #4044, #4083)
  	WiFi.disconnect(true);    // Delete SDK wifi config
  	delay(200);
  	WiFi.mode(WIFI_OFF);
	WiFi.forceSleepBegin();
	delay(100);

}
bool connectWifi(){
	//setIP(marinerUan,marinerId);
	WiFi.hostname("espMeteo");      // DHCP Hostname (useful for finding device for static lease)
  	WiFi.mode(WIFI_STA);
  	WiFi.forceSleepWake();
	delay(10);
  	WiFi.config(marinerUan, gateway, subnet,dns1); // Set static IP (2,7s) or 8.6s with DHCP  + 2s on battery
  	delay(10);
  	WiFi.begin(ssid, password);
  	delay(10);
	uint32_t wifi_initiate = millis();
  	while (WiFi.status() != WL_CONNECTED) {
    if ((millis() - wifi_initiate) > 5000L) {
      	return false;
    	}
    delay(700);
  }
  delay(10);
  return true;
}
bool connectMQTT(){
	meteoclient.setServer(mqtt_server, mqtt_port);
  	meteoclient.setCallback(callback);
  	String clientId = String(marinerId);
  	clientId += String(random(0xffff), HEX);
	delay(100);
	meteoclient.connect(clientId.c_str(),mqttUser,mqttPass);
	uint32_t wifi_initiate = millis();
  	smartDelay(100);
	while (!meteoclient.connected()){
		if ((millis() - wifi_initiate) > 5000L) {
      	return false;
    	}
		delay(700);
  	}
	delay(100);
  	return true;
}
void faiSampleDummy(){
	for (size_t i = 0; i < 4; i++)
	{
		requestSensorsValues();
		delay(10);
		storeData(i);
		delay(100);
		blinkLed(1,true);
		delay(2000);
	}
}
void setup(){
	#ifdef DEBUGMIO
	pinMode(LED_BUILTIN,OUTPUT);
  	digitalWrite(LED_BUILTIN,HIGH);
	#endif
	setWifiOff();
	Wire.begin(default_sda_pin, default_scl_pin);
	delay(10);
	uint8_t nrRecord = readEEPROM(nValuesAddr);
	delay(10);
	if(nrRecord==255) {
		nrRecord = 0;
		writeEEPROM(nValuesAddr,nrRecord); //update storage records nr on I2C eeprom
	}
	delay(10);
	#ifdef DEBUGMIO
	blinkLed(nrRecord,true);
	faiSampleDummy();
	#else
	requestSensorsValues();
	delay(10);
	#endif
	bool risp = connectWifi();
	#ifdef DEBUGMIO
		blinkLed(2,risp);
	#endif
	if(!risp){
		storeData(nrRecord);
		delay(100);
		loop();
	}
	risp = connectMQTT();
	#ifdef DEBUGMIO
		blinkLed(3,risp);
	#endif
	if(!risp) return;
  	risp = sendThing();
	#ifdef DEBUGMIO
		blinkLed(4,risp);
	#endif
	smartDelay(100);
	risp = serializeData(nrRecord);
	if(risp)
	{
		writeEEPROM(nValuesAddr,0);	//reset storage records nr on I2C eeprom
		delay(100);
	}else
	{
		storeData(nrRecord);
		delay(100);
	}
	#ifdef DEBUGMIO
		blinkLed(5,risp);
	#endif
	delay(100);
}
void loop(){
	WiFi.persistent(false);   // Solve possible wifi init errors (re-add at 6.2.1.16 #4044, #4083)
  	WiFi.disconnect(true);    // Delete SDK wifi config
  	delay(200);
  	//WiFi.setOutputPower(17);        // 10dBm == 10mW, 14dBm = 25mW, 17dBm = 50mW, 20dBm = 100mW
	WiFi.mode(WIFI_OFF);
	WiFi.forceSleepBegin();
	delay(100);
	shutDownNow();
	delay(1000);
}
void storeData(uint8_t nrRecords){
  uint16_t availAddress = 32 * nrRecords;	//MeteoData is 24 bytes ,we use 32 bytes page write
  writeStructEEPROM(availAddress);	//write struct on I2C eeprom
  nrRecords++; //add record's nr
  writeEEPROM(nValuesAddr,nrRecords); //update storage records nr on I2C eeprom
}
void callback(char* topic, byte* payload, unsigned int length){}
void shutDownNow(){
	//this tell to attiny to power down ESP

	//DEBPRINT("spegniti");
	Wire.begin(default_sda_pin, default_scl_pin);
	delay(100);
	Wire.beginTransmission (2);
  	Wire.write (20);
  	Wire.endTransmission(true);
}
uint8_t requestSensorsValues(){
	//bool sStop=true;
	Wire.requestFrom(SLAVE_ATTINY_ADDRESS, 2);    // request 2 bytes from slave device #2---- Wire.requestFrom (SLAVE_ADDRESS, responseSize);
	uint8_t i=0;
	while (Wire.available()){
		dati[i] = Wire.read();    // receive a byte as character
		i++;
  	}
	voltage = (dati[1]<<8) | dati[0];
	currmet.battery=voltage;
	delay(100);
  	uint8_t res= bm(currmet);									// read BMP080 values
	res= bm(currmet);									// read BMP080 values
	return res;
}
bool serializeData(uint8_t nrRecords){
	DynamicJsonDocument doc(2048);
	JsonArray jsonHum = doc.createNestedArray("hum");
	JsonArray jsonTemp = doc.createNestedArray("temp");
	JsonArray jsonPress = doc.createNestedArray("press");
	JsonArray jsonBat = doc.createNestedArray("bat");
	delay(10);
	jsonHum.add(currmet.humidityBMP);
	jsonTemp.add(currmet.temperatureBMP);
	jsonPress.add(currmet.externalPressure);
	jsonBat.add(currmet.battery);
	delay(10);
	for (int i =  nrRecords-1 ; i >= 0 ; i--){
		readStructEEPROM(32 * i); // read I2C eeprom
		delay(10);
		jsonHum.add(met.humidityBMP);
		jsonTemp.add(met.temperatureBMP);
		jsonPress.add(met.externalPressure);
		jsonBat.add(met.battery);
		delay(100);
	}
	String s;
	serializeJson(doc, s);
	delay(100);
	HTTPClient http;
	delay(10);
	http.begin(espClient,post_serverJSON);
	delay(100);
	int httpResponseCode = http.PUT(s);
	delay(100);
	http.end();  //Free resources
	delay(100);
	if(httpResponseCode==200)return true;
	return false;
	
}
/*uint8_t printWEBJSON(uint8_t nrRecords) {//timeAvailable -> live mesaures
 	DEBUG_PRINT("nr record: "+ String(nrRecords));
	 if(nrRecords>0){
		StaticJsonBuffer<2000> JSONbuffer;
		JsonObject& JSONencoder = JSONbuffer.createObject();
		JsonArray& jsonHum = JSONencoder.createNestedArray("hum");
		JsonArray& jsonTemp = JSONencoder.createNestedArray("temp");
		JsonArray& jsonPress = JSONencoder.createNestedArray("press");
		JsonArray& jsonBat = JSONencoder.createNestedArray("bat");
		delay(100);
		nrRecords--;
		for (int i =  nrRecords ; i >= 0 ; i--){
			readStructEEPROM(32 * i); // read I2C eeprom
			delay(10);
			jsonHum.add(met.humidityBMP);
			jsonTemp.add(met.temperatureBMP);
			jsonPress.add(met.externalPressure);
			jsonBat.add(met.battery);
		}
		
		smartDelay(100);
		jsonHum.add(met.humidityBMP);
		jsonTemp.add(met.temperatureBMP);
		jsonPress.add(met.externalPressure);
		jsonBat.add(met.battery);
		int httpResponseCode=0;
		String s="";
		JSONencoder.prettyPrintTo(s);
		smartDelay(100);
		HTTPClient http;
		http.begin(espClient,post_serverJSON);
		httpResponseCode = http.PUT(s);
		delay(100);
		Serial.println(s);
		delay(100);
		http.end();  //Free resources
		if(httpResponseCode==200){
			//DEBPRINT(httpResponseCode);   //Print return code
			////DEBPRINT(response);           //Print request answer
			writeEEPROM(nValuesAddr,0); //reset storage records nr on I2C eeprom
			return 0;
		}else{
			//DEBPRINT("Error on sending POST: ");
			//DEBPRINT(httpResponseCode);
			return 1;
		}
	}
	else{
		//Serial.begin(9600);
		smartDelay(100);
    String s =String("GET /meteofeletto/swpi_logger.php?hum=" + String(met.humidityBMP) +
    +"&&pwd=" + webpass +
    +"&&temp=" + String(met.temperatureBMP) +
    +"&&press=" + String(met.externalPressure) +
    +"&&bat=" + String(met.battery) +
    + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
    smartDelay(100);
    //Serial.println(s);
    if (espClient.connect(host, httpPort))
		{
			espClient.println(s);
			smartDelay(100);
		}
  }
  return 2;
}
*/
bool sendThing(){
	StaticJsonDocument<256> doc;
	
	doc["topic"] = "Terrazza";
	doc["Hum"] =currmet.humidityBMP;
  	doc["Temp"] =currmet.temperatureBMP;
  	doc["Press"] =currmet.externalPressure;
	char JSONmessageBuffer[256];
	smartDelay(100);
	bool check =0;
	serializeJson(doc, JSONmessageBuffer);
	smartDelay(100);
	for (uint8_t i = 0; i < 3; i++)
	{
		check=meteoclient.publish(extSensTopic,JSONmessageBuffer,true);
		if(check) break;
		smartDelay(100);
	}
	smartDelay(100);
	meteoclient.disconnect();
	return check;
}
void smartDelay(unsigned long ms){
  unsigned long start = millis();
  do
  {
		meteoclient.loop();
    delay(10);
	} while (millis() - start < ms);
}
