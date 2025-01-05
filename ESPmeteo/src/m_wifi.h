// m_wifi.h
#pragma once
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include <ESP8266Ping.h>
#include "debugutils.h"
#include "topic.h"
#include "password.h"
#include "myIP.h"
WiFiClient espClient;
PubSubClient mqttClient(espClient);
const char *remote_host = "www.google.com";
namespace wifiK
{
	void smartDelay(unsigned long ms)
	{
		unsigned long start = millis();
		do
		{

			mqttClient.loop();
			delay(10);
		} while (millis() - start < ms);
	}
	
	uint8_t ackReceived = 0; // Variabile per tracciare se abbiamo ricevuto l'ACK
	// 0 = errore, 1 = ack , 2 = end

	void callback(char *topic, byte *payload, unsigned int length)
	{
		if (strcmp(topic, ackMeteoTopic) == 0)
		{
			// Controlla che l'ACK sia corretto
			String ackMessage = "";
			for (unsigned int i = 0; i < length; i++)
			{
				ackMessage += (char)payload[i];
			}
			Serial.println("ACK ricevuto: " + ackMessage);

			if (strcmp(ackMessage.c_str(), "ack") == 0)
			{
				ackReceived = 1;
			}
			else if (strcmp(ackMessage.c_str(), "end") == 0)
			{
				ackReceived = 2;
			}
			else
			{
				ackReceived = 0;
			}
		}
	}
	uint8_t waitForAck()
    {
        unsigned long startMillis = millis();
        while (millis() - startMillis < 5000)
        {
            smartDelay(100);
            if (ackReceived != 0)
            {
                return ackReceived;
            }
        }
        return 0;
    }
	
	bool connectMQTT()
	{
		mqttClient.setServer(mqtt_server, mqtt_port);
		mqttClient.setCallback(callback);
		String clientId = String(marinerId);
		clientId += String(random(0xffff), HEX);
		delay(100);
		mqttClient.connect(clientId.c_str(), mqttUser, mqttPass);
		uint32_t wifi_initiate = millis();
		smartDelay(100);
		while (!mqttClient.connected())
		{
			if ((millis() - wifi_initiate) > 5000L)
			{
				return false;
			}
			delay(200);
		}
		delay(100);
		mqttClient.subscribe(ackMeteoTopic);
		return true;
	}
	bool setWifiOff()
	{
		wifi_set_sleep_type(LIGHT_SLEEP_T);
		WiFi.persistent(false); // Solve possible wifi init errors (re-add at 6.2.1.16 #4044, #4083)
		WiFi.disconnect(true);	// Delete SDK wifi config
		delay(200);
		WiFi.forceSleepBegin();
		return WiFi.mode(WIFI_OFF);
	}
	bool connectWifi()
	{
		// setIP(marinerUan,marinerId);
		WiFi.hostname("espMeteo"); // DHCP Hostname (useful for finding device for static lease)
		WiFi.mode(WIFI_STA);
		WiFi.forceSleepWake();
		delay(10);
		WiFi.config(marinerUan, gateway, subnet, dns1); // Set static IP (2,7s) or 8.6s with DHCP  + 2s on battery
		delay(10);
		WiFi.begin(ssid, password);
		delay(10);
		// Serial.println("wifi");
		uint32_t wifi_initiate = millis();
		while (WiFi.status() != WL_CONNECTED)
		{
			if ((millis() - wifi_initiate) > 8000L)
			{
				return false;
			}
			delay(500);
		}
		delay(500);
		// if (!Ping.ping(remote_host))
		// {
		// 	return false;
		// }
		return true;
	}

	bool transmitMQTT(const char *_topic, char *_message)
	{
		smartDelay(100);
		return mqttClient.publish(_topic, _message);
	}

	bool transmitMQTT(const char *_topic, const uint8_t *_message, size_t length)
	{
		smartDelay(100);
		return mqttClient.publish(_topic, _message, length);
	}

	bool transmitHTTP(char *meteoPayload)
	{
		HTTPClient http;
		smartDelay(10);
		http.begin(espClient, post_serverJSON);
		smartDelay(100);
		int httpResponseCode = http.PUT(meteoPayload);
		smartDelay(100);

		String payload = ""; // Variabile per memorizzare la risposta del server
		if (httpResponseCode > 0)
		{
			// La richiesta è stata eseguita con successo, otteniamo la risposta
			payload = http.getString();
			DEBUG_PRINT("HTTP Response code: ");
			DEBUG_PRINT(httpResponseCode);
			DEBUG_PRINT("Server response: ");
			DEBUG_PRINT(payload);
		}
		else
		{
			// Errore durante la richiesta
			DEBUG_PRINT("Errore HTTP: ");
			DEBUG_PRINT(httpResponseCode);
		}

		http.end(); // Free resources
		delay(100);

		// Valutazione del risultato
		if (httpResponseCode == 200)
			return true;

		return false;
	}
}