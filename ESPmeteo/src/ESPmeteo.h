#ifndef espmeteo_h
#define espmeteo_h
#include <cxonfig.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "I2C_Anything.h"
#include <PubSubClient.h> //mqtt library
#include <ArduinoJson.h>
#include "okbmpdhtweb.h"
#include "wifimio.h"
#include "eeprommio.h"
#include "debugutils.h"
#include <math.h>
//#define DEBUGMIO
//#define DEBUGMQTT
HTTPClient http;
WiFiClient c;
PubSubClient client(mqtt_server, mqtt_port, c);
uint8_t connLAN();
bool printWEB(bool timeAvailable);
void requestSensorsValues();
void sendData(uint8_t nrRecords);
void smartDelay(unsigned long ms);
void callback(char* topic, byte* payload, unsigned int length);
void printMqtt();
void printMqttLog(String message);
void storeData(uint8_t nrRecords);
void shutDownNow();
void reconnect();
#endif
