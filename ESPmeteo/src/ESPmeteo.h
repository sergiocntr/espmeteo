#ifndef espmeteo_h
#define espmeteo_h
#include <ESP8266WiFi.h>
#include "debugutils.h"
#include <cxonfig.h>
#include "topic.h"
#include "password.h"
#include "myIP.h"
#include <Arduino.h>
#include "I2C_Anything.h"
#include <PubSubClient.h> //mqtt library
#include <ArduinoJson.h>
#include "okbmpdhtweb.h"
#include "eeprommio.h"
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
HTTPClient http;
WiFiClient c;
PubSubClient client(c);
IPAddress ip;
const uint16_t versione = 4;
const char* mqttID = "marinerUan";
bool reconnect();
char printWEBJSON(uint8_t records);
char requestSensorsValues();
void smartDelay(unsigned long ms);
bool sendThing();
void storeData(uint8_t nrRecords);
void shutDownNow();
void callback(char* topic, byte* payload, unsigned int length);
uint8_t checkForUpdates();
void user_init();
void preinit();
#endif
