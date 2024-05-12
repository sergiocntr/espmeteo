#pragma once
#include <Arduino.h>
#include <ESP8266WiFi.h>
#define DEBUGMIO
#include "debugutils.h"
#include <Wire.h>
#include <SPI.h>
#include "topic.h"
#include "password.h"
#include "myIP.h"
#include "I2C_Anything.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266Ping.h>
WiFiClient espClient;
PubSubClient meteoclient(espClient);
// ESP-01 SDA - SCL pin
const int default_sda_pin = 0;
const int default_scl_pin = 2;
const char *remote_host = "www.google.com";
uint16_t voltage = 0; // voltage get from attiny
uint8_t dati[2];      // attiny low and high voltage byte
// I2C eeprom stuff
const int SLAVE_ATTINY_ADDRESS = 2; //
const int SLAVE_ADDRESS = 0X50;     // classic I2C EEPROM address
uint16_t nValuesAddr;
#include "okbmpdhtweb.h"
#include "eeprommio.h"
uint8_t requestSensorsValues();
bool sendThing();
bool serializeData(uint8_t nrRecords);
void shutDownNow();
#include <helper.h>
#include <m_wifi.h>