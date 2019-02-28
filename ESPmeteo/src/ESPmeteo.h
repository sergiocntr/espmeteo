#ifndef espmeteo_h
#define espmeteo_h
#include "debugutils.h"
#include <cxonfig.h>
#include "topic.h"
#include "password.h"
//#include "myFunctions.h"
#include "myIP.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "I2C_Anything.h"
#include <PubSubClient.h> //mqtt library
#include <ArduinoJson.h>
#include "okbmpdhtweb.h"
#include "eeprommio.h"
HTTPClient http;
WiFiClient c;
PubSubClient client(c);
IPAddress ip;
const char* mqttID;
void reconnect();
bool printWEBJSON(uint8_t records);
void requestSensorsValues();
void smartDelay(unsigned long ms);
void sendThing();
void storeData(uint8_t nrRecords);
void shutDownNow();
void callback(char* topic, byte* payload, unsigned int length);
#endif
