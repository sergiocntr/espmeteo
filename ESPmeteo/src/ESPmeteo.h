// ESPmeteo.h
#ifndef ESPMETEO_H
#define ESPMETEO_H

#include <Arduino.h>
#include <ESP8266Ping.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <Wire.h>

// 1. Base definitions and config
#include "config.h"
#include "debugutils.h"
#include "impostazioni.h"
#include "m_errors.h"

// 2. Utility and Communication
#include "helper.h"
#include "m_wifi.h"

// 3. Data Storage and Sensors
#include "c_circBuff.h"
#include "c_sensors.h"
#include "eeprommio.h"

/**
 * @brief Sends a ping to the ATtiny to reset its timeout counter.
 */
void keepAlive();

#endif