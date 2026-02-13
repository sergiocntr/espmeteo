/**
 * @file helper.h
 * @brief Utility functions for serialization, logging, and error handling.
 */

#pragma once

#include "impostazioni.h"
#include "m_wifi.h"
#include <Arduino.h>
#include <Wire.h>

namespace helpMet {
/**
 * @brief Serializes a structure into a byte buffer.
 */
template <typename T> size_t serialize(const T &data, uint8_t *buffer) {
  memcpy(buffer, &data, sizeof(T));
  return sizeof(T);
}

/**
 * @brief Deserializes a byte buffer into a structure.
 */
template <typename T> void deserialize(const uint8_t *buffer, T &data) {
  memcpy(&data, buffer, sizeof(T));
}

/**
 * @brief Custom bufferization for meteoData to ensure specific field alignment.
 */
void bufferizeData(meteoData *mData, uint8_t *l_buffer) {
  int offset = 0;

  memcpy(l_buffer + offset, &mData->deviceID, sizeof(mData->deviceID));
  offset += sizeof(mData->deviceID);

  memcpy(l_buffer + offset, &mData->humidityBMP, sizeof(mData->humidityBMP));
  offset += sizeof(mData->humidityBMP);

  memcpy(l_buffer + offset, &mData->temperatureBMP,
         sizeof(mData->temperatureBMP));
  offset += sizeof(mData->temperatureBMP);

  memcpy(l_buffer + offset, &mData->externalPressure,
         sizeof(mData->externalPressure));
  offset += sizeof(mData->externalPressure);

  memcpy(l_buffer + offset, &mData->battery, sizeof(mData->battery));
  offset += sizeof(mData->battery);

  uint8_t padding[5] = {0};
  memcpy(l_buffer + offset, padding, sizeof(padding));
  offset += sizeof(padding);

  memcpy(l_buffer + offset, &mData->counter, sizeof(mData->counter));
  offset += sizeof(mData->counter);

  memcpy(l_buffer + offset, &mData->checksum, sizeof(mData->checksum));
}

char str[256]; // Global string buffer for formatting

/**
 * @brief Reusable LED blinker for status signaling.
 */
void blinkLed(uint8_t count, bool fast = false) {
  uint16_t delayMs = fast ? 250 : 500;
  for (uint8_t i = 0; i < count; i++) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(delayMs);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(delayMs);
  }
}

void printBufferLog(eeCircRecord eBuffer) {
#ifdef DEBUGMIO
  sprintf(str, "eBuffer - Wipe: 0x%02X, Flag: 0x%02X, Addr: %u, Count: %u",
          eBuffer.wipeFlag, eBuffer.flag, eBuffer.pageAddress,
          eBuffer.recordCount);
  Serial.println(str);
  wifiK::transmitMQTT(logTopic, str);
#endif
}

void printCurmetLog(eeCircRecord eBuffer) {
  sprintf(str, "Telemetry - Wipe: 0x%02X, Flag: 0x%02X, Addr: %u, Count: %u",
          eBuffer.wipeFlag, eBuffer.flag, eBuffer.pageAddress,
          eBuffer.recordCount);

  wifiK::transmitMQTT(logMeteoTopic, str);
#ifdef DEBUGMIO
  Serial.println(str);
  delay(10);
#endif
}

void printMeteoData(meteoData *mData) {
#ifdef DEBUGMIO
  Serial.printf(
      "MeteoData - Hum: %d, Temp: %d, Pres: %d, Batt: %u, Cnt: %u, Check: %u\n",
      mData->humidityBMP, mData->temperatureBMP, mData->externalPressure,
      mData->battery, mData->counter, mData->checksum);
#endif
}

void printHexBuffer(const uint8_t *buffer, size_t length) {
#ifdef DEBUGMIO
  for (size_t i = 0; i < length; i++) {
    Serial.printf("0x%02X ", buffer[i]);
    delay(1);
  }
  Serial.println();
#endif
}

/**
 * @brief Centralized error handler logic.
 */
void handleProgramError(ErrorState &error) {
#ifdef DEBUGMIO
  switch (error) {
  case BME280_CONNECTION_FAILED:
    DEBUG_PRINT("BME280 connection failed.");
    break;
  case BME280_DATA_ERROR:
    DEBUG_PRINT("Invalid data from BME280.");
    break;
  case ATTINY_BYTE_NUM_ERROR:
    DEBUG_PRINT("ATtiny I2C byte count mismatch.");
    break;
  case ATTINY_DATA_ERROR:
    DEBUG_PRINT("Invalid data from ATtiny.");
    break;
  case EEPROM_ERROR:
    DEBUG_PRINT("EEPROM read/write error.");
    break;
  case WIFI_CONNECTION_ERROR:
    DEBUG_PRINT("WiFi connection error.");
    break;
  case MQTT_CONNECTION_ERROR:
    DEBUG_PRINT("MQTT connection error.");
    break;
  case NO_ERROR:
    DEBUG_PRINT("System status OK.");
    break;
  default:
    DEBUG_PRINT("Unknown error state.");
    break;
  }
#else
  blinkLed(static_cast<uint8_t>(error), true);
#endif
}
} // namespace helpMet