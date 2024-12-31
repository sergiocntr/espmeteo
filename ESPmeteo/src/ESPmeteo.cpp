// ESPmeteo.cpp
#include "ESPmeteo.h"
char str[100];
void printBufferLog()
{
#ifdef DEBUGMIO
	// str[100];  // Assicurati che la dimensione sia abbastanza grande per contenere la stringa formattata
	sprintf(str, "eBuffer - wipeFlag: 0x%02X, flag: 0x%02X, pageAddress: %u, recordCount: %u",
			eBuffer.wipeFlag, eBuffer.flag, eBuffer.pageAddress, eBuffer.recordCount);
	Serial.println(str);				// Stampa sulla seriale
	wifiK::transmitMQTT(logTopic, str); // Pubblica su MQTT
	wifiK::smartDelay(100);				// Ritardo per evitare flooding
#endif
}
void printCurmetLog()
{
#ifdef DEBUGMIO
	// str[100];  // Assicurati che la dimensione sia abbastanza grande per contenere la stringa formattata
	sprintf(str, "Curmet - wipeFlag: 0x%02X, flag: 0x%02X, pageAddress: %u, recordCount: %u",
			eBuffer.wipeFlag, eBuffer.flag, eBuffer.pageAddress, eBuffer.recordCount);
	Serial.println(str);				// Stampa sulla seriale
	wifiK::transmitMQTT(logTopic, str); // Pubblica su MQTT
	wifiK::smartDelay(100);				// Ritardo per evitare flooding
#endif
}
void blinkLed(uint8_t volte, bool l)
{
	uint16_t tempo;

	if (l)
	{
		tempo = 250;
	}
	else
	{
		tempo = 500;
	}
	for (uint8_t i = 0; i < volte; i++)
	{
		digitalWrite(LED_BUILTIN, LOW);
		delay(tempo);
		digitalWrite(LED_BUILTIN, HIGH);
		delay(tempo);
	}
}
void faiSampleDummy()
{
	uint8_t i = 1;
	while (eBuffer.recordCount < 17)
	{

		// Simulazione dati sensore
		currmet.temperatureBMP = random(20, 30) + random(0, 99) / 100.0;
		currmet.humidityBMP = random(40, 60) + random(0, 99) / 100.0;
		currmet.externalPressure = random(970, 1041); // Pressione atmosferica simulata
		currmet.battery = random(3950, 4451);		  // Livello batteria simulato
		currmet.checksum = 0xAB;					  // checksum
		currmet.counter = eBuffer.recordCount;						  // counter
		delay(1500);

		circBuf::appendRecordToEeprom(currmet);
		circBuf::readLastPageAddressAndRecordCount();
		printBufferLog();
		i++;
	}
}

bool sendMultiple(uint8_t nrRecords) // MANDA AL SERVER TUTTI I RECORD PIU L'ULTIMO
{
	DynamicJsonDocument doc(1024);
	JsonArray jsonHum = doc.createNestedArray("hum");
	JsonArray jsonTemp = doc.createNestedArray("temp");
	JsonArray jsonPress = doc.createNestedArray("press");
	JsonArray jsonBat = doc.createNestedArray("bat");

	delay(10);

	// Aggiungi i dati correnti (curmet) per MQTT
	jsonHum.add(currmet.humidityBMP);
	jsonTemp.add(currmet.temperatureBMP);
	jsonPress.add(currmet.externalPressure);
	jsonBat.add(currmet.battery);

	char JSONmessageBuffer[1024];
	wifiK::smartDelay(100);
	bool check = false;

	// Serializza i dati correnti per MQTT
	serializeJson(doc, JSONmessageBuffer);
	wifiK::smartDelay(100);

	// Invia i dati correnti tramite MQTT
	for (uint8_t i = 0; i < 3; i++)
	{
		check = wifiK::transmitMQTT(extSensTopic, JSONmessageBuffer);
		if (check)
			break;
		// wifiK::smartDelay(100);
	}

	wifiK::smartDelay(100);
	// meteoclient.disconnect();

	// Se ci sono record memorizzati (nrRecords > 0), aggiungili al documento per HTTP
	if (nrRecords > 0)
	{
		for (int i = nrRecords - 1; i >= 0; i--)
		{
			readStructEEPROM(32 * i); // Leggi i dati dall'EEPROM
			delay(10);
			jsonHum.add(met.humidityBMP);
			jsonTemp.add(met.temperatureBMP);
			jsonPress.add(met.externalPressure);
			jsonBat.add(met.battery);
			delay(100);
		}
	}

	// Serializza il documento completo per HTTP (composto dai dati correnti e dai record)
	serializeJson(doc, JSONmessageBuffer);
	wifiK::smartDelay(100);

	// Invia i dati al server HTTP
	HTTPClient http;
	delay(10);
	http.begin(espClient, post_serverJSON);
	delay(100);
	int httpResponseCode = http.PUT(JSONmessageBuffer); // Usa i dati serializzati per HTTP
	delay(100);
	http.end(); // Libera le risorse
	delay(100);

	// Verifica il risultato della richiesta HTTP
	if (httpResponseCode == 200)
	{
		return true;
	}

	return false;
}
void setup()
{
	delay(1000);
#ifdef DEBUGMIO
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);
	Serial.begin(9600);
	Serial.println("partito");
#endif
	bool risp = wifiK::setWifiOff(); // APPENA ACCESO SPENGO IL WIFI PER RISPARMIARE CORRENTE
	Wire.begin(default_sda_pin, default_scl_pin);
	delay(10);

	circBuf::readLastPageAddressAndRecordCount();
	delay(10);
	printBufferLog();
	delay(10);
	faiSampleDummy();

	delay(10);

	risp = wifiK::connectWifi();
	if (!risp)
	{ // SE NON HO CONNESSIONE MEMORIZZO I DATI SULLA MEMORIA I2C

		sprintf(str, "La connessione WIFI è fallita.");
		DEBUG_PRINT(str);
		return;
	}

	risp = wifiK::connectMQTT();

	if (!risp)
	{

		sprintf(str, "La connessione MQTT è fallita.");
		DEBUG_PRINT(str);
		return;
	}

	circBuf::readAndSendStoredRecordFromEeprom();
}

void loop()
{
#ifdef DEBUGMIO
	sprintf(str, "Spengo");
	Serial.println(str);
	wifiK::transmitMQTT(logTopic, str);
	wifiK::smartDelay(100);
#endif
	wifiK::smartDelay(100);
	wifiK::setWifiOff();
	delay(100);
	shutDownNow();
}

void shutDownNow()
{

	Wire.begin(default_sda_pin, default_scl_pin);
	delay(100);
	Wire.beginTransmission(2);
	Wire.write(20);
	Wire.endTransmission(true);
}
uint8_t requestSensorsValues()
{
	// bool sStop=true;
	Wire.requestFrom(SLAVE_ATTINY_ADDRESS, 2); // request 2 bytes from slave device #2---- Wire.requestFrom (SLAVE_ADDRESS, responseSize);
	uint8_t i = 0;
	while (Wire.available())
	{
		dati[i] = Wire.read(); // receive a byte as character
		i++;
	}
	voltage = (dati[1] << 8) | dati[0];
	currmet.battery = voltage;
	delay(100);
	uint8_t res = bm(currmet); // read BMP080 values
	res = bm(currmet);		   // read BMP080 values
	return res;
}
