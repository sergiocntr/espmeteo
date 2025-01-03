//impostazioni.h
#pragma once
const int default_sda_pin = 0;
const int default_scl_pin = 2;


// I2C eeprom stuff
const int SLAVE_ATTINY_ADDRESS = 2; //
const int EEPROM_I2C_ADDRESS = 0X50;     // classic I2C EEPROM address
struct meteoData{
	float humidityBMP; 			//4 byte
	float temperatureBMP; 		//4 byte
	float externalPressure; 	//4 byte
  	uint16_t battery;			//2 byte
	uint8_t counter;       		// 1 byte: nr of record
	uint8_t checksum;   		// 1 byte: checksum per la validità dei dati
};
struct eeCircRecord {
    uint8_t wipeFlag = 0xFF;        // Default: 0xFF
    uint8_t flag = 0xAA;            // Default: 0xAA
    uint16_t pageAddress = 0x0000;       // Default: 0
    uint16_t recordCount = 0;       // Default: 0
};