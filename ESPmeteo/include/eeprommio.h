// eeprommio.h
#pragma once
#include "EEPROMError.h"
#include "helper.h"
#include "impostazioni.h"
namespace nEEPROM {

bool writeToEEPROM(uint16_t startAddr, const uint8_t *buffer, uint8_t length) {
  Wire.beginTransmission(EEPROM_I2C_ADDRESS);
  Wire.write((uint8_t)(startAddr >> 8));   // Byte alto dell'indirizzo
  Wire.write((uint8_t)(startAddr & 0xFF)); // Byte basso dell'indirizzo

  for (uint8_t i = 0; i < length; i++)
    Wire.write(buffer[i]);

  uint8_t check = Wire.endTransmission();
  delay(100); // Attesa per completare la scrittura
  if (check != 0) {
    Serial.println("Errore nella scrittura I²C!");
    return false;
  }

  return true;
}

bool readFromEEPROM(uint16_t startAddr, uint8_t *buffer, uint8_t length) {
  Wire.beginTransmission(EEPROM_I2C_ADDRESS);
  Wire.write((startAddr >> 8) & 0xFF); // MSB
  Wire.write(startAddr & 0xFF);        // LSB
  if (Wire.endTransmission() != 0) {
    return false; // Errore nella trasmissione
  }
  Wire.requestFrom(static_cast<uint8_t>(EEPROM_I2C_ADDRESS),
                   static_cast<uint8_t>(length));
  for (uint8_t i = 0; i < length && Wire.available(); i++) {
    buffer[i] = Wire.read();
  }
  return true;
}

bool readEEPROMDataWithRetry(uint16_t address, uint8_t *buffer, uint8_t length,
                             uint8_t maxRetries = 3) {
  uint8_t attempts = 0;
  bool success = false;

  while (attempts < maxRetries) {
    if (readFromEEPROM(address, buffer, length)) {
      success = true;
      break;
    }

    attempts++;
    Serial.printf("Errore nella lettura dei dati dalla EEPROM, tentativo %d\n",
                  attempts);
    delay(100); // Pausa breve prima del prossimo tentativo
  }

  return success;
}

bool writeEEPROMDataAndVerify(uint16_t address, const uint8_t *buffer,
                              uint8_t length) {

  // Scrivi i dati nella EEPROM
  writeToEEPROM(address, buffer, length);

  // Leggi i dati dalla EEPROM per verificarli
  uint8_t readBuffer[length];

  readFromEEPROM(address, readBuffer, length);

  // Confronta i dati byte per byte

  for (uint8_t i = 0; i < length; i++) {
    if (buffer[i] != readBuffer[i]) {
      ERROR_PRINT(EEPROMError_className, EEPROMError, EEPROMError::FATAL_ERROR);
      return false;
    }
  }
  Serial.println("Dati scritti correttamente");
  helpMet::printHexBuffer(buffer, length);

  Serial.printf("Indirizzo = 0x%04X\n", address);

  return true;
}
//////// SOLO PER RESETTARE MEMORIA
void resetMemory() {
  const uint16_t nValuesAddr = 0x0000; // Indirizzo memoria di partenza
  const uint8_t defaultValue = 0xDD;   // Valore predefinito per il reset
  const uint16_t byteToErase = 512;    // Indirizzo memoria di partenza
  const int transferSize = 32;         // Valore predefinito per il reset

  DEBUG_PRINT("reset Memoria ....");

  // Scrittura dei valori nell'EEPROM
  for (uint16_t i = 0; i < byteToErase;
       i += transferSize) { // Scrittura a blocchi di 16 byte (esempio)
    Wire.beginTransmission(EEPROM_I2C_ADDRESS);

    // Imposta l'indirizzo corrente
    Wire.write((uint8_t)((nValuesAddr + i) >> 8)); // Byte alto dell'indirizzo
    Wire.write(
        (uint8_t)((nValuesAddr + i) & 0xFF)); // Byte basso dell'indirizzo

    // Scrive 16 byte per ciclo
    for (uint8_t j = 0; j < transferSize; j++) {
      Wire.write(defaultValue);
    }

    // Conclude la trasmissione e verifica eventuali errori
    uint8_t result = Wire.endTransmission();
    if (result != 0) {
      DEBUG_PRINT("Errore nella scrittura I2C");
      return; // Esce se c'è un errore
    }

    delay(50); // Attesa per completare la scrittura
  }
  // Scrittura dei valori nell'EEPROM
  for (uint16_t i = 0; i < byteToErase;
       i += transferSize) { // Scrittura a blocchi di 16 byte (esempio)
    Wire.beginTransmission(EEPROM_I2C_ADDRESS);
    Wire.write((uint8_t)(nValuesAddr >> 8));   // Byte alto dell'indirizzo
    Wire.write((uint8_t)(nValuesAddr & 0xFF)); // Byte basso dell'indirizzo
    Wire.endTransmission();

    Wire.requestFrom(EEPROM_I2C_ADDRESS,
                     transferSize); // Richiede transferSize byte

    DEBUG_PRINT("Lettura valori:");
    for (uint16_t i = 0; i < transferSize; i++) {
      if (Wire.available()) {
        uint8_t value = Wire.read();
        Serial.print("0x");
        Serial.print(value, HEX);
        Serial.print(" ");
      } else {
        DEBUG_PRINT("Errore: dati non disponibili");
        Serial.print("nrByte: ");
        Serial.print(i);
        break;
      }
    }
    Serial.println(); // Va a capo alla fine della stampa
  }
}
} // namespace nEEPROM
