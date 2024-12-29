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

void sendSingleData()
{
	// Simulazione dati sensore
	float temperature = random(20, 30) + random(0, 99) / 100.0;
	float humidity = random(40, 60) + random(0, 99) / 100.0;
	int pres = random(970, 1041); // Pressione atmosferica simulata
	int bat = random(3950, 4451); // Livello batteria simulato

	DynamicJsonDocument doc(128);
	JsonArray jsonHum = doc.createNestedArray("hum");
	JsonArray jsonTemp = doc.createNestedArray("temp");
	JsonArray jsonPress = doc.createNestedArray("press");
	JsonArray jsonBat = doc.createNestedArray("bat");
	delay(10);
	jsonHum.add(humidity);
	jsonTemp.add(temperature);
	jsonPress.add(pres);
	jsonBat.add(bat);

	char meteo_buffer[128];
	size_t bytesWritten = serializeJson(doc, meteo_buffer);

	// Controllo se la serializzazione ha avuto successo
	if (bytesWritten == 0)
	{
		// Gestione dell'errore
		DEBUG_PRINT("Errore: la serializzazione JSON è fallita.");
		return; // O esegui l'azione necessaria in caso di errore
	}

	// Invio MQTT
	if (wifiK::transmitMQTT(logTopic, meteo_buffer))
	{
		DEBUG_PRINT("MQTT: Dati singoli inviati con successo!");
	}
	else
	{
		DEBUG_PRINT("MQTT: Errore invio dati singoli!");
	}
	wifiK::smartDelay(100);
	if (wifiK::transmitHTTP(meteo_buffer))
	{
		DEBUG_PRINT("HTTP: Dati singoli inviati con successo!");
	}
	else
	{
		DEBUG_PRINT("HTTP: Errore invio dati singoli!");
	}
}
bool sendMultipleData()
{
	// Simulazione gruppo di dati
	DynamicJsonDocument doc(512);
	JsonArray jsonHum = doc.createNestedArray("hum");
	JsonArray jsonTemp = doc.createNestedArray("temp");
	JsonArray jsonPress = doc.createNestedArray("press");
	JsonArray jsonBat = doc.createNestedArray("bat");

	float temperature;
	float humidity;
	int pres; // Pressione atmosferica simulata
	int bat;  // Livello batteria simulato

	for (int i = 0; i < 5; i++)
	{

		temperature = random(20, 30) + random(0, 99) / 100.0;
		humidity = random(40, 60) + random(0, 99) / 100.0;
		pres = random(970, 1041); // Pressione atmosferica simulata
		bat = random(3950, 4451); // Livello batteria simulato
		jsonHum.add(humidity);
		jsonTemp.add(temperature);
		jsonPress.add(pres);
		jsonBat.add(bat);
		wifiK::smartDelay(100);
	}

	char meteo_buffer[512];

	size_t bytesWritten = serializeJson(doc, meteo_buffer);

	// Controllo se la serializzazione ha avuto successo
	if (bytesWritten == 0)
	{
		// Gestione dell'errore
		DEBUG_PRINT("Errore: la serializzazione JSON è fallita.");
		return false; // O esegui l'azione necessaria in caso di errore
	}

	// Se tutto va bene, puoi procedere con l'invio o altre operazioni
	DEBUG_PRINT("JSON serializzato correttamente:");
	wifiK::smartDelay(100);
	DEBUG_PRINT(meteo_buffer);
	wifiK::smartDelay(100);

	// Invio MQTT
	if (wifiK::transmitMQTT(logTopic, meteo_buffer))
	{
		wifiK::smartDelay(100);
		DEBUG_PRINT("MQTT: Gruppo di dati inviato con successo!");
		
	}
	else
	{
		wifiK::smartDelay(100);
		DEBUG_PRINT("MQTT: Errore invio gruppo di dati!");
		
	}
	wifiK::smartDelay(100);
	if (wifiK::transmitHTTP(meteo_buffer))
	{
		DEBUG_PRINT("HTTP: Dati multipli inviati con successo!");
		return true;
	}
	else
	{
		DEBUG_PRINT("HTTP: Errore invio dati multipli!");
		return false;
	}
}
void setup()
{
	delay(2000);

	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);
	Serial.begin(9600);
	delay(1000);
	DEBUG_PRINT("partito");

	bool risp = wifiK::setWifiOff(); // APPENA ACCESO SPENGO IL WIFI PER RISPARMIARE CORRENTE
	if (!risp)
	{ // SE NON HO CONNESSIONE MEMORIZZO I DATI SULLA MEMORIA I2C

		sprintf(str, "WIFI Sleep è fallito.");
		DEBUG_PRINT(str);

		//return;
	}
	sprintf(str, "WIFI OFF : ok!");
	DEBUG_PRINT(str);
	delay(20);
	risp = wifiK::connectWifi();

	if (!risp)
	{ // SE NON HO CONNESSIONE MEMORIZZO I DATI SULLA MEMORIA I2C

		sprintf(str, "La connessione WIFI è fallita.");
		DEBUG_PRINT(str);
		return;
	}

	sprintf(str, "WIFI OK");
	DEBUG_PRINT(str);

	risp = wifiK::connectMQTT();

	if (!risp)
	{

		sprintf(str, "La connessione MQTT è fallita.");
		DEBUG_PRINT(str);
		return;
	}

	sprintf(str, "MQTT OK");
	DEBUG_PRINT(str);
	sprintf(str, "Trasmissione Singolo Record...");
	DEBUG_PRINT(str);
	sendSingleData();
	wifiK::smartDelay(100);
	sprintf(str, "Trasmissione Multipli Record...");
	DEBUG_PRINT(str);
	sendMultipleData();
	wifiK::smartDelay(100);
	wifiK::setWifiOff(); // APPENA ACCESO SPENGO IL WIFI PER RISPARMIARE CORRENTE
}
void loop()
{
}
