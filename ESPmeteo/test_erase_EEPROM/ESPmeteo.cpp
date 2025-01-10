//ESPmeteo.cpp
#include "ESPmeteo.h"
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
void setup()
{
	delay(2000);
#ifdef DEBUGMIO
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);
	Serial.begin(9600);
	Serial.println("partito");
#endif
	setWifiOff(); // APPENA ACCESO SPENGO IL WIFI PER RISPARMIARE CORRENTE
	delay(2000);
	Wire.begin(default_sda_pin, default_scl_pin);
	delay(10);
	//resetMemory();
	//delay(10);
	resetCircBuffer();

}
void loop()
{

}
