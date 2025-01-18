// ESPmeteo.cpp
/*
Errore di lettura dalla sonda BME280.
Impossibilità di connettersi a MQTT (timeout, server non disponibile, credenziali errate).
Memoria EEPROM piena o errori di scrittura.
Buffer circolare pieno o errore nel gestirne la capacità.
Errore di trasmissione via MQTT (rete, server MQTT, formato dei dati).
Errore nel riavvio e nel recupero dei dati dalla memoria EEPROM.
Errore nel comando di spegnimento (risorse non rilasciate correttamente).
*/
#include "ESPmeteo.h"
extern "C"
{
#include "user_interface.h"
}

void printFreeRAM(const char *label)
{
	Serial.print(label);
	Serial.print(" - Free heap: ");
	Serial.println(ESP.getFreeHeap());
}
void shutDownNow()
{

	Wire.begin(default_sda_pin, default_scl_pin);
	Wire.beginTransmission(2);
	Wire.write(20);
	Wire.endTransmission(true);
	delay(50);
}

#define DEBUGMIO
void setup()
{

#ifdef DEBUGMIO
	delay(1000);
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);
	Serial.begin(9600);
#endif
	wifiK::setWifiOff(); // APPENA ACCESO SPENGO IL WIFI PER RISPARMIARE CORRENTE
	Wire.begin(default_sda_pin, default_scl_pin);

	ErrorState check = NO_ERROR;

	SensorDataHandler sHandler;
	EepromDataHandler eHandler;

	sHandler.initialize(check);

	if (static_cast<uint8_t>(check) > 0)
	{
		helpMet::handleProgramError(check);
		return;
	}

	
	meteoData mdata;

//#define DUMMY_RECORD
#if defined DUMMY_RECORD
	uint8_t NRECORD = 5;
	DEBUG_PRINT("Reset memory...")
	nEEPROM::resetMemory();
	eHandler.initialize(true);
	DEBUG_PRINT("Inserimento Record...");
	for (size_t i = 0; i < NRECORD; i++)
	{
		check = sHandler.getDummyRecord(&mdata);
		eHandler.appendRecordToEeprom(&mdata);
		delay(50);
	}

#else

	eHandler.initialize(false);
	check = sHandler.getRecord(&mdata);

#endif

	if (static_cast<uint8_t>(check) > 0)
	{
		helpMet::handleProgramError(check);
		return;
	}

	wifiK::connectWifi(check);

	if (static_cast<uint8_t>(check) > 0)
	{
		helpMet::handleProgramError(check);
		eHandler.appendRecordToEeprom(&mdata);
		return;
	}
	delay(10);

	wifiK::connectMQTT(check);

	if (static_cast<uint8_t>(check) > 0)
	{
		helpMet::handleProgramError(check);
		eHandler.appendRecordToEeprom(&mdata);
		return;
	}

	wifiK::smartDelay(100);
	uint16_t storedrecords = eHandler.storedRecords();
	if (storedrecords == 0) // Se non ci sono messaggi memorizzati ,spedisco questo messaggio
	{
		sprintf(helpMet::str, "Spedisco singolo record.");
		DEBUG_PRINT(helpMet::str);
		uint8_t buff[32];
		size_t sizemsg = helpMet::serialize(mdata, buff);
		Eeprom_job ejob = NO_JOB;
		bool result = wifiK::sendWithRetry(buff, sizemsg, ejob);
		if (!result)
		{
			eHandler.appendRecordToEeprom(&mdata);
			sprintf(helpMet::str, "La trasmissione MQTT è fallita, memorizzo dati su EEPROM.");
			DEBUG_PRINT(helpMet::str);
		}
	}
	else // spedisco tutto il blocco
	{
		sprintf(helpMet::str, "Spedisco multipli record: %u.", storedrecords);
		DEBUG_PRINT(helpMet::str);
		eHandler.appendRecordToEeprom(&mdata); // ?? BUG 17: NON ABILITARE ,RECORD GIA SCRITTO
		wifiK::smartDelay(100);
		eHandler.readAndSendStoredRecordFromEeprom();
	}
}

void loop()
{
#ifdef DEBUGMIO

	sprintf(helpMet::str, "Spengo");
	Serial.println(helpMet::str);
	wifiK::transmitMQTT(logTopic, helpMet::str);

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
