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
EepromDataHandler eHandler;
SensorDataHandler sHandler;

#define DEBUGMIO

void putDataToEeprom()
{
	eHandler.initialize(true);

	meteoData mdata;

	uint16_t storedrecords = eHandler.storedRecords();

	for (size_t i = 0; i < 20; i++)
	{
		check = sHandler.getDummyRecord(&mdata);

		if (static_cast<uint8_t>(check) > 0)
		{
			helpMet::handleProgramError(check);
			return;
		}

		eHandler.appendRecordToEeprom(&mdata);
	}
}
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

	sHandler.initialize(check);

	if (static_cast<uint8_t>(check) > 0)
	{
		helpMet::handleProgramError(check);
		return;
	}

	Serial.println("Usa 'e' per ERASE, 'w' per WRITE, s per SEND.");
}

void loop()
{
	// Controllo se c'è un input disponibile
	if (Serial.available() > 0)
	{
		char input = Serial.read(); // Legge il carattere inserito

		// Esegui l'azione in base al carattere ricevuto
		switch (input)
		{
		case 'e':
			nEEPROM::resetMemory();
			break;
		case 'w':
			putDataToEeprom();
			break;
		case 's':
			wifiK:
			putDataToEeprom();
			break;
		default:
			Serial.println("Comando non riconosciuto. Usa 'w' o 's'.");
			break;
		}
	}
}

/*
recupera quanti record sono memorizzati
recupera dati meteo
metti il numero dei record
fai checksum
serializza

*/
