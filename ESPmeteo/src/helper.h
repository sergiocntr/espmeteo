#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "m_wifi.h"
#include "impostazioni.h"
namespace helpMet
{
    template <typename T>
    size_t serialize(const T &data, uint8_t *buffer)
    {
        memcpy(buffer, &data, sizeof(T)); // Copia i dati nella memoria del buffer
        return sizeof(T);
    }
    template <typename T>
    void deserialize(const uint8_t *buffer, T &data)
    {
        memcpy(&data, buffer, sizeof(T)); // Copia i dati dal buffer alla struttura
    }

    float truncate(float val, int decimals = 2)
    {
        float factor = pow(10, decimals);
        return floor(val * factor) / factor;
    }

    char str[100];
    void printBufferLog(eeCircRecord eBuffer)
    {
#ifdef DEBUGMIO
        // str[100];  // Assicurati che la dimensione sia abbastanza grande per contenere la stringa formattata
        sprintf(str, "eBuffer - wipeFlag: 0x%02X, flag: 0x%02X, pageAddress: %u, recordCount: %u",
                eBuffer.wipeFlag, eBuffer.flag, eBuffer.pageAddress, eBuffer.recordCount);
        Serial.println(str);                // Stampa sulla seriale
        wifiK::transmitMQTT(logTopic, str); // Pubblica su MQTT
        wifiK::smartDelay(100);             // Ritardo per evitare flooding
#endif
    }
    void printCurmetLog(eeCircRecord eBuffer)
    {
#ifdef DEBUGMIO
        // str[100];  // Assicurati che la dimensione sia abbastanza grande per contenere la stringa formattata
        sprintf(str, "Curmet - wipeFlag: 0x%02X, flag: 0x%02X, pageAddress: %u, recordCount: %u",
                eBuffer.wipeFlag, eBuffer.flag, eBuffer.pageAddress, eBuffer.recordCount);
        Serial.println(str);                // Stampa sulla seriale
        wifiK::transmitMQTT(logTopic, str); // Pubblica su MQTT
        wifiK::smartDelay(100);             // Ritardo per evitare flooding
#endif
    }

    void printHexBuffer(const uint8_t* buffer, size_t length) {
        for (size_t i = 0; i < length; i++) {
          if (buffer[i] < 0x10) {
            Serial.print("0"); // Aggiunge uno zero per valori sotto 0x10
          }
          Serial.print(buffer[i], HEX); // Stampa il valore in esadecimale
          Serial.print(" "); // Separa i valori con uno spazio
        }
        Serial.println(); // Va a capo alla fine del buffer
      }

}