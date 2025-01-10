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
// #include "c_wifi.h"

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
    uint8_t l_buffer[64];        // Buffer di dati
    eeCircRecord circRecord; // Struttura record

    void prepareBuffer(uint8_t *rawBuffer, uint16_t totalRecordToTransmit)
    {
        rawBuffer[TRANSFER_SIZE] = static_cast<uint8_t>(totalRecordToTransmit >> 8);
        rawBuffer[TRANSFER_SIZE + 1] = static_cast<uint8_t>(totalRecordToTransmit & 0xFF);
    }

    bool sendWithRetry(uint8_t *rawBuffer, size_t bufferSize)
    {
        uint8_t ackStatus;
        uint8_t retries = 2; // Numero massimo di ritrasmissioni

        while (retries-- > 0)
        {
            bool sent = sendDataViaMQTT(rawBuffer, bufferSize);

            if (sent)
            {
                ackStatus = wifiK::waitForAck(); // Chiama una sola volta

                sprintf(helpMet::str, "ackStatus:  %u .", ackStatus);
                DEBUG_PRINT(helpMet::str);

                // Gestione dell'ACK in base al valore ricevuto
                if (ackStatus == 1)
                { // ack
                    Serial.println("Dati parziali inviati e confermati con successo!");
                    ERROR_PRINT(MQTTError_className, MQTTError, MQTTError::ACK_OK);
                    return true; // Successo
                }
                else if (ackStatus == 2)
                { // end
                    ERROR_PRINT(MQTTError_className, MQTTError, MQTTError::END_OK);
                    return true; // Successo
                }
                else if (ackStatus == 3)
                { // failed
                    ERROR_PRINT(MQTTError_className, MQTTError, MQTTError::ACK_FAILED);
                    Serial.println("Errore nella trasmissione, ritrasmettendo...");
                    wifiK::smartDelay(100);
                    continue; // Ritrasmetti il record
                }
                else if (ackStatus == 0)
                { // fatal
                    ERROR_PRINT(EEPROMError_className, EEPROMError, EEPROMError::FATAL_ERROR);
                    Serial.println("Errore fatale nella EEPROM");
                    initializeCircularRecord(); // Aggiorna il puntatore della EEPROM
                    return false;               // Errore fatale
                }
            }
            else
            {
                ERROR_PRINT(MQTTError_className, MQTTError, MQTTError::PUBLISH_FAILED);
                Serial.println("Errore persistente nell'invio dei dati!");
                return false; // Errore nell'invio
            }
        }

        // Se arriviamo qui, significa che abbiamo esaurito i tentativi senza successo
        return false;
    }

    bool sendDataViaMQTT(uint8_t *rawBuffer, size_t bufferLength)
    {
        uint8_t nrTry = 0;
        bool sent = false;
        while (!sent && nrTry < 2)
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

    // void updateEEPROM(uint16_t &pageAddress, uint16_t &recordCount) // aggiorna il buffer circolare ,6 byte
    // {
    //     circRecord.pageAddress += RECORD_SIZE;
    //     circRecord.recordCount--;

    //     if (circRecordAddress)
    //     {
    //         circRecord.pageAddress = BUFFER_CIRCULAR_START + ((circRecord.pageAddress - BUFFER_CIRCULAR_START + TRANSFER_SIZE) % (BUFFER_CIRCULAR_END - BUFFER_CIRCULAR_START));
    //     }

    //     helpMet::serialize(circRecord, buffer);
    //     nEEPROM::writeEEPROMDataAndVerify(circRecordAddress, buffer, BUFFER_SIZE);
    // }

    bool initializeCircularRecord()
    {
        DEBUG_PRINT("Initialize!!");
        //uint8_t l_buffer[sizeof(eeCircRecord)];
        circRecordAddress = BUFFER_CIRCULAR_START;
        circRecord.wipeFlag = 0xFF;
        circRecord.flag = DEFAULT_FLAG;
        circRecord.recordCount = 0x00;
        circRecord.pageAddress = 0x0000;

        helpMet::serialize(circRecord, l_buffer);
        return nEEPROM::writeEEPROMDataAndVerify(circRecordAddress, l_buffer, sizeof(eeCircRecord));
    }
    // Funzione privata che viene chiamata per leggere l'ultimo indirizzo e il contatore del record
    bool readLastPageAddressAndRecordCount()
    {
        //uint8_t l_buffer[sizeof(eeCircRecord)]; // Dichiarazione corretta della variabile

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
        return initializeCircularRecord();
    }

public:
    // Costruttore della classe
    EepromDataHandler()
    {
    }

    bool initialize()
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

    uint16_t storedRecords()
    {
        return circRecord.recordCount;
    }

    bool appendRecordToEeprom(meteoData *mData)
    {
        initialize();
        DEBUG_PRINT("HI.");
        helpMet::printCurmetLog(circRecord);
        DEBUG_PRINT("HI.");

        DEBUG_PRINT("HI.");
        uint16_t freeAddr = circRecord.pageAddress + (circRecord.recordCount * RECORD_SIZE);
        DEBUG_PRINT("HI.");
#if defined DEBUGMIO
        Serial.printf("freeAddr = 0x%04X\n", freeAddr);
#endif
        DEBUG_PRINT("HI.");
        if ((freeAddr + RECORD_SIZE) > BUFFER_CIRCULAR_START)
        {
            Serial.println("Errore: Memoria EEPROM DATI piena!");
            ERROR_PRINT(EEPROMError_className, EEPROMError, EEPROMError::EE_FULL);
            return false;
        }
        DEBUG_PRINT("HI.");
        circRecord.recordCount++;
        DEBUG_PRINT("HI.");
        Serial.println(circRecord.recordCount);
        DEBUG_PRINT("HI.");
        mData->counter = circRecord.recordCount;
        DEBUG_PRINT("HI.");
        int offset = 0;
        DEBUG_PRINT("HI.");
        //uint8_t l_buffer[RECORD_SIZE];
        // Scrittura dei dati nel buffer utilizzando memcpy
        memcpy(l_buffer + offset, &mData->humidityBMP, sizeof(mData->humidityBMP));
        offset += sizeof(mData->humidityBMP);
        DEBUG_PRINT("HI.");
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

        //helpMet::printHexBuffer(l_buffer, RECORD_SIZE);

        nEEPROM::writeEEPROMDataAndVerify(freeAddr, l_buffer, RECORD_SIZE);

        helpMet::serialize(circRecord, l_buffer);

        return nEEPROM::writeEEPROMDataAndVerify(circRecordAddress, l_buffer, BUFFER_SIZE);
    }
    /*
    Punti deboli o migliorabili
Mancanza di timestamp o ID univoco:

I record non hanno timestamp o ID, quindi eventuali duplicati o fuori sequenza (es. in caso di pacchetti ritrasmessi disordinati) potrebbero diventare un problema. Aggiungere un identificatore univoco per ciascun record aiuterebbe a rilevare queste situazioni.
Conservazione dei record corrotti:

Sebbene i record corrotti siano contrassegnati, la loro presenza nel buffer potrebbe essere problematica per applicazioni che richiedono dati perfetti. Potremmo introdurre una fase di validazione finale per eliminare o isolare questi record.
Limiti nella tolleranza agli errori consecutivi:

Il limite di "due messaggi consecutivi errati" potrebbe essere troppo rigido in ambienti con interferenze elevate o problemi temporanei di rete. Una soglia configurabile basata su applicazioni reali potrebbe essere più adatta.
Resilienza generale:

Non è implementato un meccanismo di timeout per gestire situazioni in cui un client smette di rispondere o continua a inviare dati errati.
    */

    void readAndSendStoredRecordFromEeprom()
    {
        uint8_t rawBuffer[TRANSFER_SIZE + 2];
        uint16_t totalRecordToTransmit = circRecord.recordCount;
        sprintf(helpMet::str, "Spedisco %u record.", totalRecordToTransmit);
        DEBUG_PRINT(helpMet::str);

        for (uint16_t i = 0; i < totalRecordToTransmit; i += MAX_STRUCT)
        {
            delay(10);
            uint16_t recordAddress = circRecord.pageAddress + (i * TRANSFER_SIZE);

            if (!nEEPROM::readEEPROMDataWithRetry(recordAddress, rawBuffer, TRANSFER_SIZE))
            {
                Serial.println("Errore persistente nella lettura dei dati dalla EEPROM!");
                ERROR_PRINT(EEPROMError_className, EEPROMError, EEPROMError::FATAL_ERROR);
                return;
            }

            prepareBuffer(rawBuffer, totalRecordToTransmit);

            helpMet::printHexBuffer(rawBuffer, sizeof(rawBuffer));

            // Utilizza la funzione di ritrasmissione
            if (!sendWithRetry(rawBuffer, sizeof(rawBuffer)))
            {
                Serial.println("Errore durante la trasmissione dopo diversi tentativi.");
                initializeCircularRecord();
                wifiK::smartDelay(100);
                return; // Esci in caso di errore
            }
        }

        ERROR_PRINT(CIRBUFFError_className, CIRBUFFError, CIRBUFFError::SUCCESS);
    }
};