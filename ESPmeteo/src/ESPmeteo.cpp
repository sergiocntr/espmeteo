#include "ESPmeteo.h"
//#define DEBUGMIO
void setup()
{
	#ifdef DEBUGMIO
	Serial.begin(9600);
  //setupOTA();
	delay(5000);
	#endif
	WiFi.mode(WIFI_OFF);
	WiFi.forceSleepBegin();
	delay(100);
	DEBUG_PRINT("Eccoci!");
	DEBUG_PRINT("eccodi cacchio");
	client.setServer(mqtt_server, 8883);
  client.setCallback(callback);
	Wire.begin(default_sda_pin, default_scl_pin);
	//have we records stored in I2C ?
	uint8_t value = readEEPROM(nValuesAddr);
	if(value==255) {
		value = 0;
		writeEEPROM(nValuesAddr,value); //update storage records nr on I2C eeprom
	}
	DEBUG_PRINT("ci sono ");
	//READ SENSORS
	requestSensorsValues();
	DEBUG_PRINT("presi valori");
	//CHECK INTERNET CONNECTION
	uint8_t check = connLAN(); 		//check == 1 -> connected to local WIFI
	if(check==0){
		DEBUG_PRINT("NO LAN memorizzo e chiudo");
		storeData(value);
		shutDownNow();
	}
	DEBUG_PRINT("conn lan ok! ");

  check = connINTERNET(c); 		//check == 1 -> connected to INTERNET
	//if WIFI available and records stored, send them to server
	if(check == 1 && value > 0)
	{
		DEBUG_PRINT("internet ok -> invio dati memorizzati " + String(value));
		sendData(value);
	}

	//data section
	if(check == 1){	//  if local WIFI connection OK send data without save to I2C
		DEBUG_PRINT("internet ok -> mando dati live");
    printWEB(true); // send data to server (true = get time from web server (live record))
		//delay(1000);
		DEBUG_PRINT("1");
		reconnect();
		DEBUG_PRINT("2");
		//smartDelay(100);
		//client.loop();
		DEBUG_PRINT("3");
		printMqtt();
		client.disconnect();
		delay(100);
		//setup();
		//shutDownNow();
	}else
	{
		DEBUG_PRINT("NO LAN memorizzo e chiudo");
		//storeData(value);
		//shutDownNow();
	}
}
void smartDelay(unsigned long ms){
  unsigned long start = millis();
  do
  {
    client.loop();
		//ArduinoOTA.handle();
  } while (millis() - start < ms);
}
void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}
void storeData(uint8_t nrRecords){
	uint16_t availAddress = 32 * nrRecords;	//MeteoData is 24 bytes long so..
	//compile struct object with current data
	met.battery = voltage;
	met.humidityDHT22 = humidityDHT22;
	met.temperatureDHT22 = temperatureDHT22;
	met.externalPressure = p0;
	writeStructEEPROM(availAddress);	//write struct on I2C eeprom
	nrRecords++; //add record's nr
	writeEEPROM(nValuesAddr,nrRecords); //update storage records nr on I2C eeprom
}
void shutDownNow(){
	DEBUG_PRINT("spegniti");
  delay(500);
	Wire.begin(default_sda_pin, default_scl_pin);
	delay(500);
	Wire.beginTransmission (2);
  Wire.write (20);
  Wire.endTransmission(true);
}
void loop(){


  shutDownNow();
	delay(3000);
}
//data
void requestSensorsValues(){

  for (int i=0; i <= 2; i++){
    Wire.requestFrom(2, 2);    // request 2 bytes from slave device #2---- Wire.requestFrom (SLAVE_ADDRESS, responseSize);
    dati[i] = Wire.read();    // receive a byte as character
  }
  voltage = (dati[1]<<8) | dati[0];
  bm(temperatureDHT22,humidityDHT22,p0);									// read BMP080 values
	retmet.battery = voltage;
	retmet.humidityDHT22 = humidityDHT22;
	retmet.temperatureDHT22 = temperatureDHT22;
	retmet.externalPressure = p0;
}
//WIFI

void printWEB(bool timeAvailable) {//timeAvailable -> live mesaures
	if(!timeAvailable)
	{
		voltage = met.battery;
    humidityDHT22 = met.humidityDHT22 ;
    temperatureDHT22 = met.temperatureDHT22 ;
    p0 = met.externalPressure ;
	}else
	{
		voltage = retmet.battery;
		humidityDHT22 = retmet.humidityDHT22 ;
		temperatureDHT22 = retmet.temperatureDHT22 ;
		p0 = retmet.externalPressure ;
	}
	double gamma = log(humidityDHT22 / 100) + ((17.62 * temperatureDHT22) / (243.5 + temperatureDHT22));
	dp = 243.5 * gamma / (17.62 - gamma);
	double Humidex = temperatureDHT22 + (5 * ((6.112 * pow( 10, 7.5 * temperatureDHT22/(237.7 + temperatureDHT22))*humidityDHT22/100) - 10))/9;
	//DEBUG_PRINT("connected");
	// Make a HTTP request:
	String s =String("GET /meteofeletto/swpi_logger.php?temp_out=" + String(temperatureDHT22) +
	+"&&pwd=" + webpass +
	+"&&hum_out=" + String(humidityDHT22) +
	+"&&rel_pressure=" + String(p0) +
	+"&&dwew=" + String(dp) +
	+"&&humidex=" + String(Humidex) +
	+"&&voltage=" + String(voltage) +
	+"&&time=" + String(timeAvailable) +
	+ " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
	for (int i = 0; i < 10; i++) {
		if (c.connect(host, httpPort)) break;
		smartDelay(500);
	}
	c.print(s);
	c.flush();
	unsigned long timeout = millis();
	while (c.available() == 0){
		if (millis() - timeout > 5000){
			//printMqttLog("timeout OK dal server");
			DEBUG_PRINT("no risposta da meteofeletto");
			return;
		}
	}
	String line = c.readStringUntil('\n');
	if(line.compareTo("OK")) printMqttLog("inviato dati sul server: OK");
	else printMqttLog("inviato dati sul server: FAIL");

}
void sendData(uint8_t nrRecords){ // send stored I2C eeprom meteo data to web server
	for (int i = 0 ; i <= (nrRecords - 1); i++){
		readStructEEPROM(32 * i); // read I2C eeprom
		printWEB(false);// false add time from last web server record (recorded record)
	}
	writeEEPROM(nValuesAddr,0); //reset storage records nr on I2C eeprom
}
//MQTT//////////////////////////////////////////////////////////////
void printMqtt(){
		StaticJsonBuffer<300> JSONbuffer;
		JsonObject& JSONencoder = JSONbuffer.createObject();
		//dtostrf(humidityDHT22, 4, 1, Humbuffer);
		//dtostrf(temperatureDHT22, 4, 1, Tempbuffer);
		//dtostrf(p0, 4, 1, Pressbuffer);
		JSONencoder["topic"] = "Terrazza";
		JSONencoder["Hum"] = String(humidityDHT22);
		JSONencoder["Temp"] = String(temperatureDHT22);
		JSONencoder["Press"] = String(p0);
		char JSONmessageBuffer[100];
		JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
		//reconnect();
		client.publish(sensorsTopic, JSONmessageBuffer,true);
		DEBUG_PRINT("mandato dati a mqtt");

}
void printMqttLog(String message){
	StaticJsonBuffer<300> JSONbuffer;
	JsonObject& JSONencoder = JSONbuffer.createObject();
	JSONencoder["topic"] = "Log";
	JSONencoder["Log"] = message;
	char JSONmessageBuffer[100];
	JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
	//reconnect();
	client.publish(sensorsTopic, JSONmessageBuffer,true);
	DEBUG_PRINT("pubblicato log! " + message);

}
void reconnect() {
			for (char i = 0; i < 10; i++)
		{
			Serial.print("Attempting MQTT connection...");
			//(clientID, username, password, willTopic, willQoS, willRetain, willMessage)
			if (client.connect(nodeID,mqttUser,mqttPass))
			{
				Serial.println("connected");
				//conn = True;
				client.publish(logTopic, "NodeMCU Caldaia connesso");
				//client.subscribe(riscaldaTopic);
				client.loop();
				//client.subscribe(acquaTopic);
				//client.loop();
				Serial.print("OOOOOKKKK, rc=");
				Serial.print(client.state());
				break;
			}
			else
			{
				Serial.print("failed, rc=");
				Serial.print(client.state());
				Serial.println(" try again in 5 seconds");
				smartDelay(50);
			}
		}
	}
