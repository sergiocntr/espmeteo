// eeprommio.h
#pragma once
#include "impostazioni.h"
namespace nEEPROM
{

    bool writeToEEPROM(uint16_t startAddr, const uint8_t* buffer, uint8_t length)
    {
        Wire.beginTransmission(EEPROM_I2C_ADDRESS);
        Wire.write((uint8_t)(startAddr >> 8));   // Byte alto dell'indirizzo
        Wire.write((uint8_t)(startAddr & 0xFF)); // Byte basso dell'indirizzo

        for (uint8_t i = 0; i < length; i++)
            Wire.write(buffer[i]);

        uint8_t check = Wire.endTransmission();
        delay(25); // Attesa per completare la scrittura
        if (check != 0)
        {
            Serial.println("Errore nella scrittura I²C!");
            return false;
        }

        return true;
    }

    bool readFromEEPROM(uint16_t startAddr, uint8_t *buffer, uint8_t length)
    {
        Wire.beginTransmission(EEPROM_I2C_ADDRESS);
        Wire.write((startAddr >> 8) & 0xFF); // MSB
        Wire.write(startAddr & 0xFF);        // LSB
        Wire.endTransmission();

        Wire.requestFrom(EEPROM_I2C_ADDRESS, length);
        for (uint8_t i = 0; i < length && Wire.available(); i++)
        {
            buffer[i] = Wire.read();
        }

        return true;
    }

    bool readEEPROMDataWithRetry(uint16_t address, uint8_t *buffer, uint8_t length, uint8_t maxRetries = 3)
    {
        uint8_t attempts = 0;
        bool success = false;

        while (attempts < maxRetries)
        {
            if (readFromEEPROM(address, buffer, length))
            {
                success = true;
                break;
            }

            attempts++;
            Serial.printf("Errore nella lettura dei dati dalla EEPROM, tentativo %d\n", attempts);
            delay(100); // Pausa breve prima del prossimo tentativo
        }

        return success;
    }

    bool writeEEPROMDataAndVerify(uint16_t address, const uint8_t *buffer, uint8_t length)
    {

        // Scrivi i dati nella EEPROM
        writeToEEPROM(address, buffer, length);

        // Leggi i dati dalla EEPROM per verificarli
        uint8_t readBuffer[length];

        readFromEEPROM(address, readBuffer, length);

        // Confronta i dati byte per byte

        for (uint8_t i = 0; i < length; i++)
        {
            if (buffer[i] != readBuffer[i])
            {
                Serial.println("Errore nella scrittura della EEPROM: dati non corrispondono.");
                return false;
            }
        }

        return true;
    }
}
