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
#include "I2C_Anything.h"
#include "helper.h"
#include "m_wifi.h"

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
    uint8_t buffer[128];        // Buffer di dati
    eeCircRecord circRecord;    // Struttura record

    void prepareBuffer(uint8_t *rawBuffer, uint16_t totalRecordToTransmit)
    {
        rawBuffer[TRANSFER_SIZE + 1] = static_cast<uint8_t>(totalRecordToTransmit >> 8);
        rawBuffer[TRANSFER_SIZE + 2] = static_cast<uint8_t>(totalRecordToTransmit & 0xFF);
    }

    bool sendDataViaMQTT(uint8_t *rawBuffer, size_t bufferLength)
    {
        uint8_t nrTry = 0;
        bool sent = false;
        while (!sent && nrTry < 3)
        {
            sent = wifiK::transmitMQTT(meteoDataTopic, rawBuffer, bufferLength);
            if (!sent)
            {
                nrTry++;
                wifiK::smartDelay(500);
                Serial.printf("Tentativo %d fallito, riprovo...\n", nrTry);
                ERROR_PRINT(MQTTError_className, MQTTError, MQTTError::PUBLISH_FAILED);
            }
            
        }
        return sent;
    }

    void updateEEPROM(uint16_t &pageAddress, uint16_t &recordCount)
    {
        circRecord.pageAddress += RECORD_SIZE;
        circRecord.recordCount--;

        if (circRecordAddress)
        {
            circRecord.pageAddress = BUFFER_CIRCULAR_START + ((circRecord.pageAddress - BUFFER_CIRCULAR_START + TRANSFER_SIZE) % (BUFFER_CIRCULAR_END - BUFFER_CIRCULAR_START));
        }

        helpMet::serialize(circRecord, buffer);
        nEEPROM::writeEEPROMDataAndVerify(circRecordAddress, buffer, BUFFER_SIZE);
    }
    // Funzione privata che viene chiamata per leggere l'ultimo indirizzo e il contatore del record
    uint8_t readLastPageAddressAndRecordCount()
    {
        for (circRecordAddress = BUFFER_CIRCULAR_START; circRecordAddress < BUFFER_CIRCULAR_END; circRecordAddress++)
        {
            if (!nEEPROM::readEEPROMDataWithRetry(circRecordAddress, buffer, BUFFER_SIZE))
            {
                DEBUG_PRINT("Errore nel recuperare circRecordAddress");
                ERROR_PRINT(CIRBUFFError_className, CIRBUFFError, CIRBUFFError::CIRCREC_NOT_FOUND);
                return 1;
            }

            helpMet::deserialize(buffer, circRecord);

            if (circRecord.flag == DEFAULT_FLAG)
            {
#if defined DEBUGMIO
                Serial.printf("Trovato 0xAA a 0x%04X\n", circRecordAddress);
#endif
                return 0; // OK, trovato il record
            }
            delay(10);
        }

        // Memoria fresca o fine dello spazio, inizializza il buffer
        circRecordAddress = BUFFER_CIRCULAR_START;
        circRecord.wipeFlag = 0xFF;
        circRecord.flag = DEFAULT_FLAG;
        circRecord.recordCount = 0x00;
        circRecord.pageAddress = 0x0000;

        helpMet::serialize(circRecord, buffer);
        nEEPROM::writeEEPROMDataAndVerify(circRecordAddress, buffer, sizeof(eeCircRecord));

        return 0;
    }

public:
    // Costruttore della classe
    EepromDataHandler()
    {
        // Esegui automaticamente la funzione privata durante l'inizializzazione
        uint8_t result = readLastPageAddressAndRecordCount();
        if (result != 0)
        {
            DEBUG_PRINT("Errore durante l'inizializzazione del buffer circolare.");
            ERROR_PRINT(CIRBUFFError_className, CIRBUFFError, CIRBUFFError::CIRCREC_NOT_BUILD);
        }
    }
    
    uint16_t storedRecords()
    {
        return circRecord.recordCount;
    }
    bool appendRecordToEeprom(meteoData &mData)
    {
        // readLastPageAddressAndRecordCount();

        uint16_t freeAddr = circRecord.pageAddress + (circRecord.recordCount * RECORD_SIZE);

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

        mData.counter = circRecord.recordCount;

        helpMet::serialize(mData, buffer);
        nEEPROM::writeEEPROMDataAndVerify(freeAddr, buffer, RECORD_SIZE);

        helpMet::serialize(circRecord, buffer);
        return nEEPROM::writeEEPROMDataAndVerify(circRecordAddress, buffer, BUFFER_SIZE);
    }

    void readAndSendStoredRecordFromEeprom()
    {
        uint8_t rawBuffer[TRANSFER_SIZE + 3];
        uint16_t totalRecordToTransmit = circRecord.recordCount;
        sprintf(helpMet::str, "Spedisco %u record.", totalRecordToTransmit);
        DEBUG_PRINT(helpMet::str);

        for (uint16_t i = 0; i < totalRecordToTransmit; i += MAX_STRUCT)
        {
            uint16_t recordAddress = circRecord.pageAddress + (i * TRANSFER_SIZE);

            if (!nEEPROM::readEEPROMDataWithRetry(recordAddress, buffer, TRANSFER_SIZE))
            {
                Serial.println("Errore persistente nella lettura dei dati dalla EEPROM!");
                ERROR_PRINT(EEPROMError_className, EEPROMError, EEPROMError::FATAL_ERROR);
                return;
            }

            prepareBuffer(rawBuffer, totalRecordToTransmit);

            helpMet::printHexBuffer(rawBuffer, sizeof(rawBuffer));

            bool sent = sendDataViaMQTT(rawBuffer, sizeof(rawBuffer));
            if (sent)
            {
                uint8_t ackStatus = wifiK::waitForAck(); // Chiama una sola volta

                sprintf(helpMet::str, "ackStatus :  %u .", ackStatus);
                DEBUG_PRINT(helpMet::str);

                if (ackStatus == 1) // ack
                {
                    Serial.println("Dati parziali inviati e confermati con successo!");
                    ERROR_PRINT(MQTTError_className, MQTTError, MQTTError::ACK_OK);
                    updateEEPROM(circRecord.pageAddress, circRecord.recordCount);
                }
                else if (ackStatus == 2) // end
                {

                    ERROR_PRINT(MQTTError_className, MQTTError, MQTTError::END_OK);
                    updateEEPROM(circRecord.pageAddress, circRecord.recordCount);
                }
                else
                {
                    ERROR_PRINT(MQTTError_className, MQTTError, MQTTError::ACK_FAILED);
                    return;
                }
            }
            else
            {
                ERROR_PRINT(MQTTError_className, MQTTError, MQTTError::PUBLISH_FAILED);
                Serial.println("Errore persistente nell'invio dei dati!");
                return;
            }
        }

        ERROR_PRINT(CIRBUFFError_className, CIRBUFFError, CIRBUFFError::SUCCESS);
    }
};