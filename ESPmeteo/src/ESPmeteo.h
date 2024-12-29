//ESPmeteo.h
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <SPI.h>
#include "I2C_Anything.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP8266Ping.h>
#include "impostazioni.h"
#include "topic.h"
#include "password.h"
#include "myIP.h"
#include "m_wifi.h"
#include "okbmpdhtweb.h"
#include "eeprommio.h"
#include "debugutils.h"

uint8_t requestSensorsValues();
void shutDownNow();
