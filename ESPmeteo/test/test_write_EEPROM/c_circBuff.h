/*
questo fie ha tre funzioni :
- readLastPageAddressAndRecordCount(eeCircRecord &eBuffer)
    Questa torna l'indirizzo della struct con il buffer circolare
-  void appendRecordToEeprom(const meteoData &newRecord)
    questa memorizza un record
-  void readAndSendStoredRecordFromEeprom()
    questa spedisce tutto sul server
*/
#pragma once
#include <Wire.h>
#include "eeprommio.h"

#include "helper.h"
#include "impostazioni.h"

// Dati generali
static const uint16_t EEPROM_PAGE_SIZE = 64;

// Dati del buffer circolare
static const uint16_t BUFFER_SIZE = 6;
static const uint16_t BUFFER_CIRCULAR_START = 0x0FE0;
static const uint16_t BUFFER_CIRCULAR_END = (BUFFER_CIRCULAR_START + EEPROM_PAGE_SIZE - BUFFER_SIZE);
static const uint8_t DEFAULT_FLAG = 0xAA;

// Dati meteo
static const uint8_t RECORD_SIZE = 16;
static const uint8_t MAX_STRUCT = 2;
static const uint16_t TRANSFER_SIZE = (MAX_STRUCT * RECORD_SIZE);

class EepromDataHandler
{
private:
    uint16_t circRecordAddress; // Indirizzo del buffer
    uint8_t l_buffer[64];       // Buffer di dati
    eeCircRecord circRecord;    // Struttura record

    void updateCircolarRecord()
    { // ad ogni trasmissione aggiorno il buffer

        circRecord.recordCount -= 2;
        circRecord.pageAddress += TRANSFER_SIZE;

        helpMet::serialize(circRecord, l_buffer);

        nEEPROM::writeEEPROMDataAndVerify(circRecordAddress, l_buffer, BUFFER_SIZE);
    }

    bool readLastPageAddressAndRecordCount()
    {
        // uint8_t l_buffer[sizeof(eeCircRecord)]; // Dichiarazione corretta della variabile

        for (circRecordAddress = BUFFER_CIRCULAR_START; circRecordAddress < BUFFER_CIRCULAR_END; circRecordAddress++)
        {
            if (!nEEPROM::readEEPROMDataWithRetry(circRecordAddress, l_buffer, sizeof(eeCircRecord)))
            {
                DEBUG_PRINT("Errore nel recuperare circRecordAddress");
                ERROR_PRINT(CIRBUFFError_className, CIRBUFFError, CIRBUFFError::CIRCREC_NOT_FOUND);
                return false;
            }

            helpMet::deserialize(l_buffer, circRecord);

            if (circRecord.flag == DEFAULT_FLAG)
            {
#if defined DEBUGMIO
                Serial.printf("Trovato 0xAA a 0x%04X\n", circRecordAddress);
#endif
                return true; // OK, trovato il record
            }

            delay(10);
        }

        DEBUG_PRINT("Non trovato flag!");

        return false;
    }

    void prepareBuffer(uint8_t *rawBuffer, uint16_t totalRecordToTransmit)
    {
        rawBuffer[TRANSFER_SIZE + 1] = static_cast<uint8_t>(totalRecordToTransmit >> 8);
        rawBuffer[TRANSFER_SIZE] = static_cast<uint8_t>(totalRecordToTransmit & 0xFF);
    }

public:
    // Costruttore della classe
    EepromDataHandler()
    {
    }

    bool initialize(bool resetCircular)
    {
        // Esegui automaticamente la funzione privata durante l'inizializzazione
        bool result = readLastPageAddressAndRecordCount();
        if (result == false)
        {
            DEBUG_PRINT("Errore durante l'inizializzazione del buffer circolare.");
            ERROR_PRINT(CIRBUFFError_className, CIRBUFFError, CIRBUFFError::CIRCREC_NOT_BUILD);
            return false;
        }
        return true;
    }

    bool appendRecordToEeprom(meteoData *mData)
    {
        Serial.println("Adesso memorizzo il record meteodata");

        helpMet::printCurmetLog(circRecord);

        uint16_t freeAddr = circRecord.pageAddress + (circRecord.recordCount * RECORD_SIZE); // PUNTO LIBERO DELLA MEMORIA

#if defined DEBUGMIO
        Serial.printf("freeAddr = 0x%04X\n", freeAddr);
#endif

        if ((freeAddr + RECORD_SIZE) > BUFFER_CIRCULAR_START)
        {
            Serial.println("Errore: Memoria EEPROM DATI piena!");
            ERROR_PRINT(EEPROMError_className, EEPROMError, EEPROMError::EE_FULL);
            return false;
        }

        circRecord.recordCount++;

        mData->counter = circRecord.recordCount;

        int offset = 0;

        // uint8_t l_buffer[RECORD_SIZE];
        //  Scrittura dei dati nel buffer utilizzando memcpy
        memcpy(l_buffer + offset, &mData->humidityBMP, sizeof(mData->humidityBMP));
        offset += sizeof(mData->humidityBMP);

        memcpy(l_buffer + offset, &mData->temperatureBMP, sizeof(mData->temperatureBMP));
        offset += sizeof(mData->temperatureBMP);

        memcpy(l_buffer + offset, &mData->externalPressure, sizeof(mData->externalPressure));
        offset += sizeof(mData->externalPressure);

        memcpy(l_buffer + offset, &mData->battery, sizeof(mData->battery));
        offset += sizeof(mData->battery);

        memcpy(l_buffer + offset, &mData->counter, sizeof(mData->counter));
        offset += sizeof(mData->counter);

        memcpy(l_buffer + offset, &mData->checksum, sizeof(mData->checksum));
        helpMet::printMeteoData(mData);
        Serial.println("stampa buffer meteodata");

        helpMet::printHexBuffer(l_buffer, RECORD_SIZE);
        Serial.println("stampa indirizzo di meomorizzazione");
        Serial.print("0x");
        Serial.println(freeAddr, HEX); // Stampa il valore in esadecimale

        nEEPROM::writeEEPROMDataAndVerify(freeAddr, l_buffer, RECORD_SIZE); // MEMORIZZO DATI METEO NELLA EEPROM

        helpMet::serialize(circRecord, l_buffer);

        return nEEPROM::writeEEPROMDataAndVerify(circRecordAddress, l_buffer, BUFFER_SIZE); // MEMORIZZO IN NUOVO RECORD COUNT NEL BUFFER CIRCOLARE
    }

    uint16_t storedRecords()
    {
        return circRecord.recordCount;
    }

    void readAndSendStoredRecordFromEeprom()
    {
        uint8_t rawBuffer[TRANSFER_SIZE + 2];
        uint16_t totalRecordToTransmit = circRecord.recordCount;
        sprintf(helpMet::str, "Spedisco %u record.", totalRecordToTransmit);
        DEBUG_PRINT(helpMet::str);
        Eeprom_job ejob = NO_JOB;

        for (uint16_t i = 0; i < totalRecordToTransmit; i += MAX_STRUCT) // corretta lettura 64byte
        {
            delay(10);
            uint16_t recordAddress = circRecord.pageAddress + (i * TRANSFER_SIZE);

            if (!nEEPROM::readEEPROMDataWithRetry(recordAddress, rawBuffer, TRANSFER_SIZE))
            {
                Serial.println("Errore persistente nella lettura dei dati dalla EEPROM!");
                ERROR_PRINT(EEPROMError_className, EEPROMError, EEPROMError::FATAL_ERROR);
                return;
            }

            prepareBuffer(rawBuffer, (totalRecordToTransmit - i));
#ifdef DEBUGMIO
            sprintf(helpMet::str, "Leggo  dati da indirizzo: 0x%04X\n", recordAddress);
            DEBUG_PRINT(helpMet::str);

            helpMet::printHexBuffer(rawBuffer, sizeof(rawBuffer));
#endif
        }

        ERROR_PRINT(CIRBUFFError_className, CIRBUFFError, CIRBUFFError::SUCCESS);
        Serial.println("ho finito il mio lavoro");
    }
};