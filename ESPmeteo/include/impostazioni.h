// impostazioni.h
#pragma once
#include <Arduino.h>

// --- Pin Definitions ---
const int default_sda_pin = 0;
const int default_scl_pin = 2;

// --- I2C / EEPROM Configuration ---
const int SLAVE_ATTINY_ADDRESS = 2;
const int EEPROM_I2C_ADDRESS = 0x50; // Standard I2C EEPROM address

/**
 * @brief Structure for weather data telemetry.
 */
struct meteoData {
  uint8_t deviceID;         // Unique device identifier
  int16_t humidityBMP;      // Humidity (float * 128)
  int16_t temperatureBMP;   // Temperature (float * 128)
  int16_t externalPressure; // Atmospheric pressure (float * 128)
  uint16_t battery;         // Battery voltage in mV
  uint8_t padding[5];       // 5-byte padding to maintain 16-byte total size
  uint8_t counter;          // Record sequence counter
  uint8_t checksum;         // Integrity check byte
};

/**
 * @brief Structure for circular buffer management in EEPROM.
 */
struct eeCircRecord {
  uint8_t wipeFlag = 0xFF;       // Reset/Wipe state indicator
  uint8_t flag = 0xAA;           // Sync flag (0xAA)
  uint16_t pageAddress = 0x0000; // Current page pointer
  uint16_t recordCount = 0;      // Total number of stored records
};

/**
 * @brief Global program error states.
 */
enum ErrorState {
  NO_ERROR = 0,
  BME280_CONNECTION_FAILED = 1,
  BME280_DATA_ERROR = 2,
  ATTINY_BYTE_NUM_ERROR = 3,
  ATTINY_DATA_ERROR = 4,
  EEPROM_ERROR = 5,
  WIFI_CONNECTION_ERROR = 6,
  MQTT_CONNECTION_ERROR = 7,
  MQTT_LOOP_ERROR = 8
};

/**
 * @brief MQTT ACK management states.
 */
enum AckState {
  NO_ACK = 0, // No ACK: Message ignored or MQTT offline
  ACK = 1,    // Message received successfully, awaiting more
  FAILED = 2, // Checksum mismatch, needs retransmission
  END = 3,    // All messages received, cleanup possible
  ERROR = 4   // EEPROM internal error
};

/**
 * @brief Job directives after background transmissions.
 */
enum Eeprom_job {
  NO_JOB = 0,
  CONTINUE = 1,      // Continue current transmission
  UPDATE_EEPROM = 3, // Transmission successful, advance pointer
  ERASE_EEPROM = 4   // Fatal error, reset memory state
};
