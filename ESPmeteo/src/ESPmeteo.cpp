// ESPmeteo.cpp
#include "ESPmeteo.h"
char str[80];
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
	for (size_t i = 0; i < 4; i++)
	{
		requestSensorsValues();
		delay(10);
		storeData(i);
		delay(100);
		blinkLed(1, true);
		delay(2000);
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
	smartDelay(100);
	bool check = false;

	// Serializza i dati correnti per MQTT
	serializeJson(doc, JSONmessageBuffer);
	smartDelay(100);

	// Invia i dati correnti tramite MQTT
	for (uint8_t i = 0; i < 3; i++)
	{
		check = meteoclient.publish(extSensTopic, JSONmessageBuffer, true);
		if (check)
			break;
		smartDelay(100);
	}

	smartDelay(100);
	meteoclient.disconnect();

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
	smartDelay(100);

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
	delay(100);
#ifdef DEBUGMIO
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);
	Serial.begin(9600);
	Serial.println("partito");
#endif
	setWifiOff(); // APPENA ACCESO SPENGO IL WIFI PER RISPARMIARE CORRENTE
	Wire.begin(default_sda_pin, default_scl_pin);
	delay(10);
	nValuesAddr = recuperoPosMemoria(); //
#ifdef DEBUGMIO
	sprintf(str, "Trovato AA nella cella  %u", nValuesAddr);
	Serial.println(str);
#endif
	nValuesAddr++;
	uint8_t nrRecord = readEEPROM(nValuesAddr); // POSIZIONE MEMORIA AA + 1 -> POSIZIONE CON I DATI
	delay(10);
#ifdef DEBUGMIO
	sprintf(str, "Ci sono %u record memorizzati.", nrRecord);
	Serial.println(str);
#endif

	if (nrRecord == 255)
	{ // QUESTO SUCCEDE SE SI ÈÌ SPORCATA LA MEMORIA,NEL QUALE CASEO RESETTO IL VALORE A ZERO
		nrRecord = 0;
		writeEEPROM(nValuesAddr, nrRecord);
	}
	delay(10);
#ifdef DEBUGMIO
	blinkLed(3, true);
	// faiSampleDummy();
	uint8_t res = requestSensorsValues();
	sprintf(str, "requestSensorsValues ha tornato  %u .", res);
	Serial.println(str);
#else
	requestSensorsValues();
	delay(10);
#endif

	bool risp = connectWifi();
// Serial.println("2");
#ifdef DEBUGMIO
	blinkLed(2, risp);
#endif
	if (!risp)
	{ // SE NON HO CONNESSIONE MEMORIZZO I DATI SULLA MEMORIA I2C
#ifdef DEBUGMIO
		sprintf(str, "La connessione WIFI è fallita.");
		Serial.println(str);
#endif
		storeData(nrRecord);
		delay(100);
		loop();
	}
#ifdef DEBUGMIO
	sprintf(str, "WIFI OK");
	Serial.println(str);
#endif
	risp = connectMQTT();
#ifdef DEBUGMIO
	blinkLed(3, risp);
#endif
	if (!risp)
	{ // SE NON HO MQTT MEMORIZZO I DATI SULLA MEMORIA I2C
#ifdef DEBUGMIO
		sprintf(str, "La connessione MQTT è fallita.");
		Serial.println(str);
		// publish(logTopic,str);
		// smartDelay(100);
#endif
		storeData(nrRecord);
		delay(100);
		loop();
	}
#ifdef DEBUGMIO
	sprintf(str, "MQTT OK");
	Serial.println(str);
	meteoclient.publish(logTopic, str);
	smartDelay(100);
#endif
	if (nrRecord > 0)
	{ // SE MAGGIORE DI ZERO PREPARO TUTTI I RECORD DA TRASMETTERE
#ifdef DEBUGMIO
		sprintf(str, "MANDO PIU' RECORD");
		Serial.println(str);
		meteoclient.publish(logTopic, str);
		smartDelay(100);
#endif
// risp = sendThing();
#ifdef DEBUGMIO
		blinkLed(4, risp);
#endif
	}
	smartDelay(100);
	risp = sendMultiple(nrRecord);
	delay(100);

	if (risp && (nrRecord > 0))
	{
		// Ho mandato anche i record memorizzati ,bene quindi punto alla nuova cella
		writeEEPROM(nValuesAddr, 0XAA);	 // Metto AA nella  cella che adesso ha i record
		writeEEPROM(nValuesAddr - 1, 0); // Cancello AA dalla vecchia cella
// nValuesAddr++; //Punto alla nuova cella di memorizzazione record
#ifdef DEBUGMIO
		sprintf(str, "Nuova cella di memorizzazione %X", nValuesAddr);
		Serial.println(str);
		meteoclient.publish(logTopic, str);
		smartDelay(100);
#endif
	}
	else
	{
		storeData(nrRecord);
	}
}

void loop()
{
#ifdef DEBUGMIO
	sprintf(str, "Spengo");
	Serial.println(str);
	meteoclient.publish(logTopic, str);
	smartDelay(100);
#endif
	smartDelay(100);
	setWifiOff();
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
