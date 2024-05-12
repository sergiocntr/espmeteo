#include "ESPmeteo.h"
 #ifdef DEBUGMIO
char str[80];
#endif
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
	WiFi.setOutputPower(14);        // 10dBm == 10mW, 14dBm = 25mW, 17dBm = 50mW, 20dBm = 100mW
  WiFi.config(marinerUan, gateway, subnet,dns1); // Set static IP (2,7s) or 8.6s with DHCP  + 2s on battery
  delay(10);
  WiFi.begin(ssid, password);
  delay(10);
	//Serial.println("wifi");
	uint32_t wifi_initiate = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if ((millis() - wifi_initiate) > 4000L) {
      return false;
    }
    delay(200);
  }
  delay(10);
   if(!Ping.ping("www.google.it")) {
		#ifdef DEBUGMIO
			sprintf(str, "La richiesta Ping è fallita.");
      Serial.println(str);
      //publish(logTopic,str);
      //delay(100);
			#endif
     return false;
   }
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
uint16_t recuperoPosMemoria(){
	//PER AUMENTARE LA VITA DELLA MEMORIA METTO UN BUFFER CIRCOLARE
	//DOVE MEMORIZZARE IL NUMERO DI RECORD SALVATI
	//OGNI VOLTA CHE SI CANCELLA LA POSIZIONE DI MEMORIA DI SCRIVE AA INVECE DI ZERO
	//E SI USA LA CELLA DOPO

	for (uint16_t i = nInitValuesAddr; i < nFinalValuesAddr; i++)
  {
    uint8_t myval= readEEPROM(i);
    if(myval == defaultValue) 
    {
      return i;
    }
	}
	writeEEPROM(nValuesAddr,0XAA); //se non trova il valore (memoria vergine) lo scrive nella prima cella del buffer circolare
	return nValuesAddr;
}
void setup(){
	delay(1000);
	#ifdef DEBUGMIO
	pinMode(LED_BUILTIN,OUTPUT);
  	digitalWrite(LED_BUILTIN,HIGH);
		Serial.begin(9600);
		Serial.println("partito");
	#endif
	setWifiOff(); //APPENA ACCESO SPENGO IL WIFI PER RISPARMIARE CORRENTE
	Wire.begin(default_sda_pin, default_scl_pin);
	delay(10);
	nValuesAddr = recuperoPosMemoria(); //CERCO INDIRIZZO CON "AA" NELLE CELLE
	#ifdef DEBUGMIO
			sprintf(str, "Trovato AA nella cella  %u", nValuesAddr);
      Serial.println(str);
      //publish(logTopic,str);
      //delay(100);
	#endif
	nValuesAddr++; //POSIZIONE MEMORIA AA + 1 -> POSIZIONE CON I DATI -> RECUPERO NR RECORD
	uint8_t nrRecord = readEEPROM(nValuesAddr); 
	delay(10);
	#ifdef DEBUGMIO
			sprintf(str, "Ci sono %u record memorizzati.",nrRecord);
      Serial.println(str);
      //publish(logTopic,str);
      //delay(100);
			#endif

	if(nrRecord==255) { //QUESTO SUCCEDE SE SI ÈÌ SPORCATA LA MEMORIA,NEL QUALE CASEO RESETTO IL VALORE A ZERO
		nrRecord = 0;
		writeEEPROM(nValuesAddr,nrRecord); 
	}
	//Serial.println("1");
	delay(10);
	#ifdef DEBUGMIO
		blinkLed(nrRecord,true);
		//faiSampleDummy();
	
	#endif
	bool  risp = requestSensorsValues();
	if(!risp){ //SE NON HO DATI METEO
		#ifdef DEBUGMIO
			sprintf(str, "La richiesta  dati meteo è fallita.");
      Serial.println(str);
      //publish(logTopic,str);
      //delay(100);
			#endif
		 //storeData(nrRecord);
		 //Serial.println("nowifi");
		delay(100);
		loop();
	}
	risp = connectWifi();
	//Serial.println("2");
	#ifdef DEBUGMIO
		blinkLed(2,risp);
	#endif
	if(!risp){ //SE NON HO CONNESSIONE MEMORIZZO I DATI SULLA MEMORIA I2C
		#ifdef DEBUGMIO
			sprintf(str, "La connessione WIFI è fallita.");
      Serial.println(str);
      //publish(logTopic,str);
      //delay(100);
			#endif
			storeData(nrRecord);
		//Serial.println("nowifi");
		delay(100);
		loop();
	}
	#ifdef DEBUGMIO
		sprintf(str, "WIFI OK");
		Serial.println(str);
		//publish(logTopic,str);
		//delay(100);
	#endif
		delay(10);
		serializeData(nrRecord);
		delay(10);
		// #ifdef DEBUGMIO
		// 	sprintf(str, "Risposta invio UDP: %i", risp);
		// 	Serial.println("risp " + String(risp));
		// #endif
		if(nrRecord > 0)
		{
			//Ho mandato i record memorizzati ,bene quindi punto alla nuova cella
			uint8_t tries = 0;
			byte Cella= 100; //valore di fantasia
			byte pval = 100; //valore di fantasia
			
			do
			{
				if(tries > 3) 
				{
						nValuesAddr++; //se cella difettosa saliamo di una
						if(nValuesAddr == nFinalValuesAddr) nValuesAddr = nInitValuesAddr;//se siamo fine buffer ricominciamo d'accapo
						tries =0;
				}
				if(tries > 10) break; //boh qua si è rotta la memoria o c'è qualcosa che non va
				writeEEPROM(nValuesAddr,0XAA);	//Metto AA nella  cella che adesso ha i record
				writeEEPROM(nValuesAddr-1,0);	//Cancello AA dalla vecchia cella 
				Cella = readEEPROM(nValuesAddr);
				pval= readEEPROM(nValuesAddr-1);
				tries ++ ;
			} while ((Cella != 0XAA) || (pval !=0) );
			#ifdef DEBUGMIO
				sprintf(str, "Nuova cella di memorizzazione %X", nValuesAddr);
      	Serial.println(str);
      //meteoclient.publish(logTopic,str);
      #endif
			delay(10);
		}
	
	
}
void loop(){
	#ifdef DEBUGMIO
			sprintf(str, "Spengo");
      Serial.println(str);
      //meteoclient.publish(logTopic,str);
      delay(100);
	#endif
	shutDownNow();
	delay(10);
	
}
void storeData(uint8_t nrRecords){
  uint16_t availAddress = 32 * nrRecords;	//MeteoData is 24 bytes ,we use 32 bytes page write
  writeStructEEPROM(availAddress);	//write struct on I2C eeprom
  nrRecords++; //add record's nr
  writeEEPROM(nValuesAddr,nrRecords); //update storage records nr on I2C eeprom
}

void shutDownNow(){

	Wire.begin(default_sda_pin, default_scl_pin);
	Wire.beginTransmission (2);
  Wire.write (20);
  Wire.endTransmission(true);
	delay(10);

}
uint8_t requestSensorsValues(){
	//bool sStop=true;
	Wire.requestFrom(SLAVE_ATTINY_ADDRESS, 2);    // request 2 bytes from slave device #2---- Wire.requestFrom (SLAVE_ADDRESS, responseSize);
	uint8_t i=0;
	while (Wire.available()){
		dati[i] = Wire.read();    // receive a byte as character
		i++;
  	}
	currmet.battery = (dati[1]<<8) | dati[0];
	delay(100);
  	uint8_t res= bm(currmet);									// read BMP080 values
	res= bm(currmet);									// read BMP080 values
	return res;
}
void serializeData(uint8_t nrRecords){
	uint16_t docSize;
	if(nrRecords > 0) docSize = 2048;
	else docSize = 256;
	DynamicJsonDocument doc(docSize);
	doc["posmem"] = nValuesAddr;
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
	if(nrRecords > 0){
		
		for (int i =  nrRecords-1 ; i >= 0 ; i--){
			readStructEEPROM(32 * i); // read I2C eeprom
			delay(10);
			jsonHum.add(met.humidityBMP);
			jsonTemp.add(met.temperatureBMP);
			jsonPress.add(met.externalPressure);
			jsonBat.add(met.battery);
			delay(10);
		}
	}
	    // Send return packet
	#ifdef DEBUGMIO
	  // Log
		Serial.print(F("Sending to "));
		Serial.print(ipZero);
		Serial.print(F(" on port "));
		Serial.println(Meteo_UDP);
		serializeJson(doc, Serial);
	#endif
  UDP.beginPacket(ipZero,Meteo_UDP);
	delay(10);
  serializeJson(doc, UDP);
	delay(10);
  UDP.endPacket();
	delay(10);

}
