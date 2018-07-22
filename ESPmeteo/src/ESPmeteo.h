#ifndef espmeteo_h
#define espmeteo_h
#include <cconfig.h>
#include <ESP8266WiFi.h>
#include <I2C_Anything.h>
#include <PubSubClient.h> //mqtt library
#include <ArduinoJson.h>
#include <okbmpdhtweb.h>

WiFiClient c;
PubSubClient client(c);
void setup();

void loop();
//WIFI
uint8_t connLAN();
void printWEB(bool timeAvailable);
void requestSensorsValues();
void sendData(uint8_t nrRecords);
byte writeStructEEPROM(unsigned int addr);
byte readStructEEPROM(unsigned int addr);
void writeEEPROM(uint16_t eeaddress, uint8_t data );
uint8_t readEEPROM(uint16_t eeaddress );
void callback(char* topic, byte* payload, unsigned int length);
void printMqtt();
#endif
