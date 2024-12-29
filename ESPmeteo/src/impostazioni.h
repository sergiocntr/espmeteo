//impostazioni.h
#pragma once
const int default_sda_pin = 0;
const int default_scl_pin = 2;
const char *remote_host = "www.google.com";
uint16_t voltage = 0; // voltage get from attiny
uint8_t dati[2];      // attiny low and high voltage byte
// I2C eeprom stuff
const int SLAVE_ATTINY_ADDRESS = 2; //
const int SLAVE_ADDRESS = 0X50;     // classic I2C EEPROM address
uint16_t nValuesAddr;
WiFiClient espClient;
PubSubClient meteoclient(espClient);