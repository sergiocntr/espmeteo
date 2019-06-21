#ifndef espmeteo_h
#define espmeteo_h
#pragma once
//#define DEBUGMIO
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include "cxonfig.h"
//#include "debugutils.h"
#include "topic.h"
#include "password.h"
#include "myIP.h"
#include "I2C_Anything.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "okbmpdhtweb.h"
//ESP-01 SDA - SCL pin
const int default_sda_pin = 0;
const int default_scl_pin = 2;
void shutDownNow();
uint16_t voltage = 0;  //voltage get from attiny
uint8_t dati[2];       // attiny low and high voltage byte
//float humidityBMP(NAN),temperatureBMP(NAN),Humidex(NAN),p0(NAN);
//I2C eeprom stuff
const int SLAVE_ATTINY_ADDRESS = 2; //classic I2C EEPROM address
const int SLAVE_ADDRESS = 0X50; //classic I2C EEPROM address
const uint16_t nValuesAddr = 0x0FFF; //address on I2C EEPROM ,we store there how many reecords we have
//meteo data object + battery
#include "eeprommio.h"
#include <myFunctions.h>
void reconnect();
void callback(char* topic, byte* payload, unsigned int length);
uint8_t printWEBJSON(uint8_t records);
uint8_t requestSensorsValues();
void smartDelay(unsigned long ms);
bool sendThing();
void storeData(uint8_t nrRecords);
uint8_t bm(meteoData& met);
#endif
