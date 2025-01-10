// ESPmeteo.cpp
#include "ESPmeteo.h"
extern "C" {
    #include "user_interface.h"
}

void printFreeRAM(const char* label) {
    Serial.print(label);
    Serial.print(" - Free heap: ");
    Serial.println(ESP.getFreeHeap());
}
void shutDownNow()
{

	Wire.begin(default_sda_pin, default_scl_pin);
	delay(100);
	Wire.beginTransmission(2);
	Wire.write(20);
	Wire.endTransmission(true);
}
void blinkLed(uint8_t volte, bool l = false)
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
#define DEBUGMIO
void setup()
{
	delay(1000);

#ifdef DEBUGMIO
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);
	Serial.begin(9600);
#endif
	bool risp = wifiK::setWifiOff(); // APPENA ACCESO SPENGO IL WIFI PER RISPARMIARE CORRENTE
	Wire.begin(default_sda_pin, default_scl_pin);
	SensorDataHandler sHandler;
	EepromDataHandler eHandler;
	sHandler.initialize();
	eHandler.initialize();
	meteoData mdata;
	uint8_t check = 0;
	uint16_t storedrecords = eHandler.storedRecords();

#define DUMMY_RECORD
#if defined DUMMY_RECORD
check = sHandler.getRecord(&mdata);
if (check > 0) return;

eHandler.appendRecordToEeprom(&mdata);

	// sprintf(helpMet::str, "Nr record :  %u .", storedrecords);
	// DEBUG_PRINT(helpMet::str);
	//  sprintf(helpMet::str, "Spedisco multipli record.");
	//  DEBUG_PRINT(helpMet::str);
	//  eHandler.readAndSendStoredRecordFromEeprom();
	//  return;

#else

	check = sHandler.getRecord(&mdata);
	if (check > 0)
	{
		blinkLed(2, true);
		return;
	}

	storedrecords = eHandler.storedRecords();

	sprintf(helpMet::str, "Nr record :  %u .", storedrecords);
	DEBUG_PRINT(helpMet::str);

	if (storedrecords > 0)
		eHandler.appendRecordToEeprom(mdata);

	sprintf(helpMet::str, "Nr record :  %u .", storedrecords);
	DEBUG_PRINT(helpMet::str);
#endif
	DEBUG_PRINT("Hi!");
	risp = wifiK::connectWifi();
	DEBUG_PRINT("Hi!");
	if (!risp)
	{ // SE NON HO CONNESSIONE MEMORIZZO I DATI SULLA MEMORIA I2C
		ERROR_PRINT(WIFIError_className, WIFIError, WIFIError::CONNECTION_FAILED);
		sprintf(helpMet::str, "La connessione WIFI è fallita, memorizzo dati su EEPROM.");
		DEBUG_PRINT(helpMet::str);
		eHandler.appendRecordToEeprom(&mdata);
		return;
	}
	// ERROR_PRINT(WIFIError_className, WIFIError, WIFIError::SUCCESS);
	delay(10);
	DEBUG_PRINT("Hi!");
	Serial.print("Free RAM before MQTT connect: ");
	Serial.println(ESP.getFreeHeap());

	risp = wifiK::connectMQTT();
	printFreeRAM("Dopo connectMQTT");
	if (!risp)
	{
		ERROR_PRINT(MQTTError_className, MQTTError, MQTTError::CONNECTION_FAILED);
		sprintf(helpMet::str, "La connessione MQTT è fallita.");
		DEBUG_PRINT(helpMet::str);
		Serial.print("Free RAM after failed MQTT connect: ");
		Serial.println(ESP.getFreeHeap());
		return;
	}

	Serial.print("Free RAM after MQTT connect: ");
	Serial.println(ESP.getFreeHeap());

	if (!risp)
	{
		ERROR_PRINT(MQTTError_className, MQTTError, MQTTError::CONNECTION_FAILED);
		sprintf(helpMet::str, "La connessione MQTT è fallita.");
		DEBUG_PRINT(helpMet::str);
		return;
	}
	DEBUG_PRINT("Hi!");
	// ERROR_PRINT(MQTTError_className, MQTTError, MQTTError::SUCCESS);
	wifiK::smartDelay(100);
	if (storedrecords == 0) // Se non ci sono messaggi memorizzati ,spedisco questo messaggio
	{

		sprintf(helpMet::str, "Spedisco singolo record.");
		DEBUG_PRINT(helpMet::str);
		uint8_t buff[64];
		size_t sizemsg = helpMet::serialize(mdata, buff);
		bool sent = wifiK::transmitMQTT(meteoDataTopic, buff, sizemsg);
		wifiK::smartDelay(100);
		if (sent)
		{
			uint8_t ackStatus = wifiK::waitForAck(); // Chiama una sola volta

			sprintf(helpMet::str, "ackStatus :  %u .", ackStatus);
			DEBUG_PRINT(helpMet::str);

			if (ackStatus == 1) // ack
			{
				Serial.println("Errore :ritornato ack per multi massaggio!");
				ERROR_PRINT(MQTTError_className, MQTTError, MQTTError::ACK_FAILED);
			}
			else if (ackStatus == 2) // end
			{

				ERROR_PRINT(MQTTError_className, MQTTError, MQTTError::END_OK);
			}
			else
			{
				ERROR_PRINT(MQTTError_className, MQTTError, MQTTError::ACK_FAILED);
				return;
			}
		}
		else
		{
			ERROR_PRINT(MQTTError_className, MQTTError, MQTTError::PUBLISH_FAILED);

			return;
		}
	}
	else // spedisco tutto il blocco
	{
		sprintf(helpMet::str, "Spedisco multipli record.");
		DEBUG_PRINT(helpMet::str);
		// eHandler.appendRecordToEeprom(&mdata);  // BUG 17: NON ABILITARE ,RECORD GIA SCRITTO
		eHandler.readAndSendStoredRecordFromEeprom();
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
