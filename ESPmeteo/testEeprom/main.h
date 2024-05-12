#include <Arduino.h>
#include <ESP8266WiFi.h>
#define DEBUGMIO
#include "debugutils.h"
#include <Wire.h>
#include "I2C_Anything.h"
#include <wifiMqtt.hpp>
struct meteoData{
	float humidityBMP;
	float temperatureBMP;
	float externalPressure;
  	uint16_t battery;
};
meteoData met;
const int default_sda_pin = 0;
const int default_scl_pin = 2;
void shutDownNow();
uint16_t voltage = 0;//voltage get from attiny
uint8_t dati[2];// attiny low and high voltage byte
//I2C eeprom stuff
const int SLAVE_ATTINY_ADDRESS = 2; //
const int SLAVE_ADDRESS = 0X50; //classic I2C EEPROM address
const uint16_t nValuesAddr = 0x0FFF; //address on I2C EEPROM ,we store there how many reecords we have
#include "eeprommio.h"
