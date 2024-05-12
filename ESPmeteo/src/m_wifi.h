#pragma once
#include <Arduino.h>
#include "ESPmeteo.h"
void callback(char *topic, byte *payload, unsigned int length) {}
void smartDelay(unsigned long ms)
{
	unsigned long start = millis();
	do
	{

		meteoclient.loop();
		delay(10);
	} while (millis() - start < ms);
}
bool connectMQTT()
{
	meteoclient.setServer(mqtt_server, mqtt_port);
	meteoclient.setCallback(callback);
	String clientId = String(marinerId);
	clientId += String(random(0xffff), HEX);
	delay(100);
	meteoclient.connect(clientId.c_str(), mqttUser, mqttPass);
	uint32_t wifi_initiate = millis();
	smartDelay(100);
	while (!meteoclient.connected())
	{
		if ((millis() - wifi_initiate) > 5000L)
		{
			return false;
		}
		delay(200);
	}
	delay(100);
	return true;
}
void setWifiOff()
{
	wifi_set_sleep_type(LIGHT_SLEEP_T);
	WiFi.persistent(false); // Solve possible wifi init errors (re-add at 6.2.1.16 #4044, #4083)
	WiFi.disconnect(true);	// Delete SDK wifi config
	delay(200);
	WiFi.mode(WIFI_OFF);
	WiFi.forceSleepBegin();
	delay(100);
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
		if ((millis() - wifi_initiate) > 6000L)
		{
			return false;
		}
		delay(500);
	}
	delay(500);
	if (!Ping.ping(remote_host))
	{
		return false;
	}
	return true;
}