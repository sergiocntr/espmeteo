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

	

	EepromDataHandler eHandler;
	eHandler.initialize();

	uint16_t storedrecords = eHandler.storedRecords();
	Serial.printf("Ci sono %u record.", storedrecords);

	eHandler.readAndSendStoredRecordFromEeprom();
	DEBUG_PRINT('aa');
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
