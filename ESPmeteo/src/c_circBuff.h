/**
 * @file c_circBuff.h
 * @brief Circular buffer management for storing weather data in EEPROM.
 *
 * Features:
 * - Tracking last page address and record count.
 * - Appending telemetry records to external I2C EEPROM.
 * - Mass transmission of stored records once connectivity is established.
 */

#pragma once

#include "eeprommio.h"
#include "helper.h"
#include "impostazioni.h"
#include <Wire.h>

// --- EEPROM Geometry Definitions ---
static const uint16_t EEPROM_START_ADDRESS = 0x0000;
static const uint16_t EEPROM_PAGE_SIZE = 64;

// --- Circular Buffer Configuration ---
static const uint16_t BUFFER_SIZE = 6;
static const uint16_t BUFFER_CIRCULAR_START = 0x7FC0;
static const uint16_t BUFFER_CIRCULAR_END =
    (BUFFER_CIRCULAR_START + EEPROM_PAGE_SIZE - BUFFER_SIZE);
static const uint8_t DEFAULT_FLAG = 0xAA;

// --- Data Record Definitions ---
static const uint8_t RECORD_SIZE = 16;
static const uint8_t MAX_STRUCT = 4;
static const uint16_t TRANSFER_SIZE = (MAX_STRUCT * RECORD_SIZE);

class EepromDataHandler {
private:
  uint16_t circRecordAddress;
  uint8_t l_buffer[64];
  eeCircRecord circRecord;

  /**
   * @brief Moves the circular record pointer forward and resets count.
   */
  void forwardCircularRecord(uint16_t freeAddress) {
    circRecordAddress++;

    if (circRecordAddress > BUFFER_CIRCULAR_END) {
      circRecordAddress = BUFFER_CIRCULAR_START;
    }

    circRecord.pageAddress = freeAddress;
    circRecord.recordCount = 0;

    helpMet::serialize(circRecord, l_buffer);
    nEEPROM::writeEEPROMDataAndVerify(circRecordAddress, l_buffer, BUFFER_SIZE);

    // Notify via MQTT about memory pointer shift
    wifiK::transmitMQTT(meteoDataTopic, l_buffer, BUFFER_SIZE);
  }

  /**
   * @brief Prepares the buffer for transmission by adding record metadata.
   */
  void prepareBuffer(uint8_t *rawBuffer, uint16_t totalRecordToTransmit) {
    rawBuffer[TRANSFER_SIZE + 1] =
        static_cast<uint8_t>(totalRecordToTransmit >> 8);
    rawBuffer[TRANSFER_SIZE] =
        static_cast<uint8_t>(totalRecordToTransmit & 0xFF);
  }

  /**
   * @brief Resets the circular record state in EEPROM.
   */
  bool initializeCircularRecord(uint16_t address = 0) {
    DEBUG_PRINT("Initializing Circular Buffer...");
    circRecordAddress = BUFFER_CIRCULAR_START;
    circRecord.wipeFlag = 0xFF;
    circRecord.flag = DEFAULT_FLAG;
    circRecord.recordCount = 0x00;
    circRecord.pageAddress = 0x0000;

    helpMet::serialize(circRecord, l_buffer);
    wifiK::transmitMQTT(meteoDataTopic, l_buffer, BUFFER_SIZE);

    return nEEPROM::writeEEPROMDataAndVerify(circRecordAddress, l_buffer,
                                             sizeof(eeCircRecord));
  }

  /**
   * @brief Locates the active circular record by searching for the sync flag.
   */
  bool readLastPageAddressAndRecordCount() {
    for (circRecordAddress = BUFFER_CIRCULAR_START;
         circRecordAddress < BUFFER_CIRCULAR_END; circRecordAddress++) {
      if (!nEEPROM::readEEPROMDataWithRetry(circRecordAddress, l_buffer,
                                            sizeof(eeCircRecord))) {
        DEBUG_PRINT("Failed to retrieve Circular Record at address");
        ERROR_PRINT(CIRBUFFError_className, CIRBUFFError,
                    CIRBUFFError::CIRCREC_NOT_FOUND);
        return false;
      }

      helpMet::deserialize(l_buffer, circRecord);

      if (circRecord.flag == DEFAULT_FLAG) {
#if defined DEBUGMIO
        Serial.printf("Sync found at 0x%04X\n", circRecordAddress);
#endif
        return true;
      }
      delay(10);
    }

    DEBUG_PRINT("No sync flag found! Initializing new buffer.");
    return initializeCircularRecord();
  }

public:
  EepromDataHandler() {}

  /**
   * @brief Initializes the handler and discovers the circular buffer state.
   */
  bool initialize(bool resetCircular = false) {
    if (resetCircular) {
      initializeCircularRecord();
    }

    if (!readLastPageAddressAndRecordCount()) {
      DEBUG_PRINT("Circular buffer initialization failed.");
      ERROR_PRINT(CIRBUFFError_className, CIRBUFFError,
                  CIRBUFFError::CIRCREC_NOT_BUILD);
      return false;
    }
    return true;
  }

  uint16_t storedRecords() { return circRecord.recordCount; }

  /**
   * @brief Appends a new weather record to the EEPROM.
   */
  bool appendRecordToEeprom(meteoData *mData) {
    DEBUG_PRINT("Storing telemetry record...");

    uint16_t freeAddr =
        circRecord.pageAddress + (circRecord.recordCount * RECORD_SIZE);

#if defined DEBUGMIO
    Serial.printf("Target Address: 0x%04X\n", freeAddr);
#endif

    if ((freeAddr + RECORD_SIZE) > BUFFER_CIRCULAR_START) {
      freeAddr = EEPROM_START_ADDRESS;
    }

    circRecord.recordCount++;
    mData->counter = circRecord.recordCount;

    uint8_t l_buffer[RECORD_SIZE];
    helpMet::bufferizeData(mData, l_buffer);

#if defined DEBUGMIO
    helpMet::printMeteoData(mData);
    helpMet::printHexBuffer(l_buffer, RECORD_SIZE);
#endif

    nEEPROM::writeEEPROMDataAndVerify(freeAddr, l_buffer, RECORD_SIZE);
    helpMet::serialize(circRecord, l_buffer);

    // Update record count in circular pointer block
    return nEEPROM::writeEEPROMDataAndVerify(circRecordAddress, l_buffer,
                                             BUFFER_SIZE);
  }

  /**
   * @brief Reads all stored records and sends them via MQTT.
   */
  void readAndSendStoredRecordFromEeprom() {
    uint8_t rawBuffer[TRANSFER_SIZE + 2];
    uint16_t totalRecordToTransmit = circRecord.recordCount;

    sprintf(helpMet::str, "Transmitting %u stored records.",
            totalRecordToTransmit);
    DEBUG_PRINT(helpMet::str);

    Eeprom_job ejob = NO_JOB;

    for (uint16_t i = 0; i < totalRecordToTransmit; i += MAX_STRUCT) {
      delay(10);
      uint16_t recordAddress = circRecord.pageAddress + (i * RECORD_SIZE);

      if (recordAddress >= BUFFER_CIRCULAR_START) {
        recordAddress -= BUFFER_CIRCULAR_START;
      }

      if (!nEEPROM::readEEPROMDataWithRetry(recordAddress, rawBuffer,
                                            TRANSFER_SIZE)) {
        Serial.println("EEPROM persistent read error!");
        ERROR_PRINT(EEPROMError_className, EEPROMError,
                    EEPROMError::FATAL_ERROR);
        return;
      }

      prepareBuffer(rawBuffer, (totalRecordToTransmit - i));

      if (!wifiK::enableTrasmit) {
        DEBUG_PRINT("Transmission disabled, aborting.");
        return;
      }

      if (!wifiK::sendWithRetry(rawBuffer, sizeof(rawBuffer), ejob)) {
        if (ejob == ERASE_EEPROM) {
          ERROR_PRINT(EEPROMError_className, EEPROMError,
                      EEPROMError::FATAL_ERROR);
          initializeCircularRecord();
        }
        return;
      } else {
        if (ejob == UPDATE_EEPROM) {
          ERROR_PRINT(EEPROMError_className, EEPROMError, EEPROMError::SUCCESS);
          forwardCircularRecord(recordAddress + TRANSFER_SIZE);
        }
      }

      // Send a keep-alive pulse to reset the ATtiny timeout
      keepAlive();
      wifiK::smartDelay(50);
    }

    wifiK::enableTrasmit = false;
    wifiK::smartDelay(50);

    ERROR_PRINT(CIRBUFFError_className, CIRBUFFError, CIRBUFFError::SUCCESS);
    DEBUG_PRINT("Offline sync completed.");
  }
};