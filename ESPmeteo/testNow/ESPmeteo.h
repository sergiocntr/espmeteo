//test UDP 5 agosto 2022
#include <Arduino.h>
#include <ESP8266WiFi.h>
//#define DEBUGMIO
#include "debugutils.h"
#include <Wire.h>
#include <SPI.h>
#include "topic.h"
#include "password.h"
#include "myIP.h"
#include "I2C_Anything.h"
#include <ArduinoJson.h>
#include <ESP8266Ping.h>
#include <WiFiUdp.h>

// UDP
WiFiUDP UDP;

//ESP-01 SDA - SCL pin
const int default_sda_pin = 0;
const int default_scl_pin = 2;

//uint16_t voltage = 0;//voltage get from attiny
uint8_t dati[2];// attiny low and high voltage byte
//I2C eeprom stuff
const int SLAVE_ATTINY_ADDRESS = 2; //
const int SLAVE_ADDRESS = 0X50; //classic I2C EEPROM address
const uint16_t nInitValuesAddr = 0x0FE0; //indirizzo memoria di partenza del buffer circolare
const uint16_t nFinalValuesAddr = 0x0FFE; //indirizzo memoria finale del buffer circolare
const uint8_t defaultValue = 0xAA; //valore di riferimento ,la cella attuale di memoria è quella dopo
uint16_t nValuesAddr;
#include "okbmpdhtweb.h"
#include "eeprommio.h"

uint8_t printWEBJSON(uint8_t records);
uint8_t requestSensorsValues();
void storeData(uint8_t nrRecords);
bool bm(meteoData& met);
void serializeData(uint8_t nrRecords);
void shutDownNow();
