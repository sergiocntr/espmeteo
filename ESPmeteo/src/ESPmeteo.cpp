// ESPmeteo.cpp
#include "ESPmeteo.h"
//#include <ArduinoJson.h>
void shutDownNow()
{

	Wire.begin(default_sda_pin, default_scl_pin);
	delay(100);
	Wire.beginTransmission(2);
	Wire.write(20);
	Wire.endTransmission(true);
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
void faiSampleDummy(eeCircRecord &eBuffer)
{
	uint8_t i = 1;

	while (eBuffer.recordCount < 17)
	{

		// Simulazione dati sensore
		// currmet.temperatureBMP = random(20, 30) + random(0, 99) / 100.0;
		// currmet.humidityBMP = random(40, 60) + random(0, 99) / 100.0;
		// currmet.externalPressure = random(970, 1041); // Pressione atmosferica simulata
		// currmet.battery = random(3950, 4451);		  // Livello batteria simulato
		// currmet.checksum = 0xAB;					  // checksum
		// currmet.counter = 0;						  // counter
		delay(1500);

		// helpMet::calculateChecksum(currmet);
		// helpMet::serializeMeteoData(mData, buffer);
		// circBuf::appendRecordToEeprom(buffer);
		// circBuf::readLastPageAddressAndRecordCount(eBuffer);
		// helpMet::printBufferLog(eBuffer);
		i++;
	}
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

	SensorDataHandler sHandler;

	meteoData mdata;
	uint8_t check = 0;
	// Leggi i dati dal sensore BME280
	check = sHandler.getRecord(&mdata);

	if (check > 0)
		return;

	EepromDataHandler eHandler;

	uint16_t storedrecords = eHandler.storedRecords();

	sprintf(helpMet::str, "Nr record :  %u .", storedrecords);
	DEBUG_PRINT(helpMet::str);

	if (storedrecords > 0)
		eHandler.appendRecordToEeprom(mdata);

	risp = wifiK::connectWifi();
	if (!risp)
	{ // SE NON HO CONNESSIONE MEMORIZZO I DATI SULLA MEMORIA I2C

		sprintf(helpMet::str, "La connessione WIFI è fallita.");
		DEBUG_PRINT(helpMet::str);
		return;
	}

	risp = wifiK::connectMQTT();

	if (!risp)
	{

		sprintf(helpMet::str, "La connessione MQTT è fallita.");
		DEBUG_PRINT(helpMet::str);
		return;
	}
	wifiK::smartDelay(100);
	if (storedrecords == 0)
	{


		// Creare un buffer per JSON
		// StaticJsonDocument<256> doc;
		// doc["temperature"] = mdata.temperatureBMP;
		// doc["humidity"] = mdata.humidityBMP;
		// char msg[256];
		uint8_t buff[256];
		size_t sizemsg = helpMet::serialize(mdata,buff);
		//serializeJson(doc, msg, sizeof(msg));

		// Trasmettere il messaggio JSON
		check = wifiK::transmitMQTT(meteoDataTopic, buff, sizemsg);
		wifiK::smartDelay(100);
		if (check == 0)
			Serial.println("msg ok!");
	}
}

void loop()
{
#ifdef DEBUGMIO

	sprintf(helpMet::str, "Spengo");
	Serial.println(helpMet::str);
	wifiK::transmitMQTT(logTopic, helpMet::str);
	wifiK::smartDelay(100);
#endif
	wifiK::smartDelay(100);
	wifiK::setWifiOff();
	delay(100);
	shutDownNow();
}

/*
recupera quanti record sono memorizzati
recupera dati meteo
metti il numero dei record
fai checksum
serializza

*/
