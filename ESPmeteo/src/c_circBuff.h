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

    void forwarCircolarRecord()
    {

        circRecordAddress++; // nuovo indirizzo del buffer circolare
        if (circRecordAddress > BUFFER_CIRCULAR_END)
            circRecordAddress = BUFFER_CIRCULAR_START; // eventualmente torna all inizio della pagina

        circRecord.pageAddress += circRecord.recordCount * RECORD_SIZE; // imposta il primo byte libero
        circRecord.recordCount = 0;                                     // abbiamo appena svuotato la memoria

        // serializzazione e memorizzazione del record circolare

        helpMet::serialize(circRecord, l_buffer);

        nEEPROM::writeEEPROMDataAndVerify(circRecordAddress, l_buffer, BUFFER_SIZE);
    }
    void prepareBuffer(uint8_t *rawBuffer, uint16_t totalRecordToTransmit)
    {
        rawBuffer[TRANSFER_SIZE + 1] = static_cast<uint8_t>(totalRecordToTransmit >> 8);
        rawBuffer[TRANSFER_SIZE] = static_cast<uint8_t>(totalRecordToTransmit & 0xFF);
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
        // uint8_t l_buffer[sizeof(eeCircRecord)];
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
        return initializeCircularRecord();
    }

public:
    // Costruttore della classe
    EepromDataHandler()
    {
    }

    bool initialize(bool resetCircular = false)
    {
        // Esegui automaticamente la funzione privata durante l'inizializzazione
        if(resetCircular) initializeCircularRecord();
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
#if defined DEBUGMIO
        helpMet::printMeteoData(mData);
        sprintf(helpMet::str, "Stampa buffer meteodata...");
        DEBUG_PRINT(helpMet::str);

        helpMet::printHexBuffer(l_buffer, RECORD_SIZE);

        sprintf(helpMet::str, "Indirizzo di memorizzazione: 0x%02X.", freeAddr);
        DEBUG_PRINT(helpMet::str);

        helpMet::printCurmetLog(circRecord);
#endif

        nEEPROM::writeEEPROMDataAndVerify(freeAddr, l_buffer, RECORD_SIZE); // MEMORIZZO DATI METEO NELLA EEPROM

        helpMet::serialize(circRecord, l_buffer);

        return nEEPROM::writeEEPROMDataAndVerify(circRecordAddress, l_buffer, BUFFER_SIZE); // MEMORIZZO IN NUOVO RECORD COUNT NEL BUFFER CIRCOLARE
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
#if defined DEBUGMIO
        // Log seriale per i dati salienti della classe
        Serial.println("=== Informazioni statiche della classe ===");
        Serial.printf("EEPROM_PAGE_SIZE: %u\n", EEPROM_PAGE_SIZE);
        Serial.printf("BUFFER_SIZE: %u\n", BUFFER_SIZE);
        Serial.printf("BUFFER_CIRCULAR_START: 0x%04X\n", BUFFER_CIRCULAR_START);
        Serial.printf("BUFFER_CIRCULAR_END: 0x%04X\n", BUFFER_CIRCULAR_END);
        Serial.printf("DEFAULT_FLAG: 0x%02X\n", DEFAULT_FLAG);
        Serial.printf("RECORD_SIZE: %u\n", RECORD_SIZE);
        Serial.printf("MAX_STRUCT: %u\n", MAX_STRUCT);
        Serial.printf("TRANSFER_SIZE: %u\n", TRANSFER_SIZE);
        Serial.println("==========================================");
#endif
        uint8_t rawBuffer[TRANSFER_SIZE + 2];
        uint16_t totalRecordToTransmit = circRecord.recordCount;
        sprintf(helpMet::str, "Spedisco %u record.", totalRecordToTransmit);
        DEBUG_PRINT(helpMet::str);
        Eeprom_job ejob = NO_JOB;

        for (uint16_t i = 0; i < totalRecordToTransmit; i += MAX_STRUCT) // corretta lettura 64byte
        {
            delay(10);
            uint16_t recordAddress = circRecord.pageAddress + (i * RECORD_SIZE);

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

            if (!wifiK::enableTrasmit)
            {
                Serial.println(" readAndSendStoredRecordFromEeprom : transmission disabled!.");
                return;
            }

            // Utilizza la funzione di ritrasmissione
            if (!wifiK::sendWithRetry(rawBuffer, sizeof(rawBuffer), ejob))
            {
                if (ejob == ERASE_EEPROM)

                {
                    ERROR_PRINT(EEPROMError_className, EEPROMError, EEPROMError::FATAL_ERROR);
                    initializeCircularRecord();
                }
                return;
            }
            else
            {
                if (ejob == UPDATE_EEPROM)
                {
                    ERROR_PRINT(EEPROMError_className, EEPROMError, EEPROMError::SUCCESS);
                    forwarCircolarRecord();
                }
            }
            wifiK::smartDelay(50);
        }

        wifiK::enableTrasmit = false;
        wifiK::smartDelay(50);

        ERROR_PRINT(CIRBUFFError_className, CIRBUFFError, CIRBUFFError::SUCCESS);
        Serial.println("ho finito il mio lavoro");
    }
};