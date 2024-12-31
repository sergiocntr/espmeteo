#pragma once
#include <Wire.h>

#define EEPROM_PAGE_SIZE 64
#define RECORD_SIZE 16
#define BUFFER_SIZE 6
#define BUFFER_CIRCULAR_START 0x0FE0
#define BUFFER_CIRCULAR_END (BUFFER_CIRCULAR_START + EEPROM_PAGE_SIZE - BUFFER_SIZE)
#define DEFAULT_FLAG 0xAA
#define TRANSFER_SIZE 32

// Numero massimo di struct (ad esempio, potrebbe essere un limite impostato)
#define MAX_STRUCT 2

// Crea un array di meteoData
meteoData data[MAX_STRUCT];

// Buffer per inviare i dati 
uint8_t rawBuffer[MAX_STRUCT * RECORD_SIZE];
namespace circBuf
{

    void writeBufferToEEPROM(uint16_t address, const eepromBuffer &buffer)
    {
        Wire.beginTransmission(EEPROM_I2C_ADDRESS);
        Wire.write((uint8_t)(address >> 8));   // Byte alto dell'indirizzo
        Wire.write((uint8_t)(address & 0xFF)); // Byte basso dell'indirizzo

        // Scrivi la struct byte per byte
        Wire.write(buffer.wipeFlag);
        Wire.write(buffer.flag);
        Wire.write((uint8_t)(buffer.pageAddress >> 8));   // Byte alto di pageAddress
        Wire.write((uint8_t)(buffer.pageAddress & 0xFF)); // Byte basso di pageAddress
        Wire.write((uint8_t)(buffer.recordCount >> 8));   // Byte alto di recordCount
        Wire.write((uint8_t)(buffer.recordCount & 0xFF)); // Byte basso di recordCount

        Wire.endTransmission();
        delay(25); // Attesa per completare la scrittura
    }

    void readBufferFromEEPROM(uint16_t address, eepromBuffer &buffer)
    {
        Wire.beginTransmission(EEPROM_I2C_ADDRESS);
        Wire.write((uint8_t)(address >> 8));   // Byte alto dell'indirizzo
        Wire.write((uint8_t)(address & 0xFF)); // Byte basso dell'indirizzo
        Wire.endTransmission();

        Wire.requestFrom(EEPROM_I2C_ADDRESS, sizeof(eepromBuffer));
        buffer.wipeFlag = Wire.read();
        buffer.flag = Wire.read();
        buffer.pageAddress = (Wire.read() << 8) | Wire.read();
        buffer.recordCount = (Wire.read() << 8) | Wire.read();
    }

    void readLastPageAddressAndRecordCount()
    {
        for (eBufferAddress = BUFFER_CIRCULAR_START; eBufferAddress < BUFFER_CIRCULAR_END; eBufferAddress++)
        {
            readBufferFromEEPROM(eBufferAddress, eBuffer);
            uint8_t flag = eBuffer.flag;
            if (flag == DEFAULT_FLAG)
            {
                Serial.print(" Trovato 0xAA  = 0x");
                Serial.print(eBufferAddress, HEX);
                Serial.println(" ");

                return; // OK TROVATO STRUCT
            }
            delay(10);
        }
        // if not flag is found (ie fresh memory or end of space ) write it to first cell an Ox00 0X00 (page address) 0x00 0x00 (record count)

        eBufferAddress = BUFFER_CIRCULAR_START;
        eBuffer.wipeFlag = 0xFF;
        eBuffer.flag = 0xAA;

        eBuffer.recordCount = 0x00;
        eBuffer.pageAddress = 0x0000;

        writeBufferToEEPROM(eBufferAddress, eBuffer);
    }

    void appendRecordToEeprom(meteoData &newRecord)
{
    // Calcola il primo byte libero basandoti sul record count
    uint16_t freeAddr = eBuffer.pageAddress + (eBuffer.recordCount * 16);

    Serial.print(" freeAddr = 0x");
    Serial.print(freeAddr, HEX);
    Serial.println(" ");

    // Verifica se superiamo i limiti della EEPROM
    if (freeAddr + 16 > BUFFER_CIRCULAR_START) {
        Serial.println("Errore: Memoria EEPROM piena!");
        return; // Esci senza scrivere
    }

    // Incrementa il contatore dei record e aggiorna il contatore nel nuovo record
    eBuffer.recordCount++;
    newRecord.counter = eBuffer.recordCount;

    // Scrivi il record nella EEPROM
    Wire.beginTransmission(EEPROM_I2C_ADDRESS);
    Wire.write((uint8_t)(freeAddr >> 8));   // Byte alto dell'indirizzo
    Wire.write((uint8_t)(freeAddr & 0xFF)); // Byte basso dell'indirizzo
    I2C_writeAnything(newRecord);
    Wire.endTransmission();
    delay(25); // Attesa per la scrittura sulla memoria

    // Aggiorna il buffer circolare nella EEPROM
    writeBufferToEEPROM(eBufferAddress, eBuffer);
}


    void readAndSendStoredRecordFromEeprom()
    {
        // Inizia la comunicazione I2C con l'EEPROM
        Wire.beginTransmission(EEPROM_I2C_ADDRESS);

        Serial.print("Lettura da indirizzo : ");
        Serial.println(eBuffer.pageAddress, HEX);

        // Scrive l'indirizzo di partenza per il primo record
        Wire.write((uint8_t)(eBuffer.pageAddress >> 8));   // Byte alto dell'indirizzo
        Wire.write((uint8_t)(eBuffer.pageAddress & 0xFF)); // Byte basso dell'indirizzo

        Wire.endTransmission();

        // Per il numero di record, leggi ogni batch di 4 (MAX_STRUCT)
        for (uint8_t i = 0; i < eBuffer.recordCount; i += MAX_STRUCT)
        {
            // Richiesta dei dati dalla EEPROM
            Wire.requestFrom(EEPROM_I2C_ADDRESS, sizeof(rawBuffer));
            delay(25);
            // Leggi i dati nella variabile rawBuffer
            I2C_readAnything(rawBuffer); // Assicurati che questa funzione legga correttamente i dati

            // Ora, se i dati sono stati letti correttamente, inviali tramite MQTT
            bool check = wifiK::transmitMQTT(meteoDataTopic, rawBuffer, sizeof(rawBuffer));
            wifiK::smartDelay(500);

            // Aggiungi una condizione per verificare che l'invio MQTT sia riuscito
            if (check)
            {//
                Serial.println("Dati inviati con successo!");
                Serial.print("Dati inviati: ");
                for (size_t i = 0; i < sizeof(rawBuffer); i++)
                {
                    Serial.printf("0x%02X ", rawBuffer[i]);
                }
                Serial.println();
                delay(10);
            }
            else
            {
                Serial.println("Errore nell'invio dei dati!");
            }
        }
    }
}
