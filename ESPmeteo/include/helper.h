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

    char str[256];

    void blinkLed(uint8_t volte, bool l = false)
    {
        uint16_t tempo;

        if (l)
        {
            tempo = 250;
        }
        else
        {
            tempo = 500;
        }
        for (uint8_t i = 0; i < volte; i++)
        {
            digitalWrite(LED_BUILTIN, LOW);
            delay(tempo);
            digitalWrite(LED_BUILTIN, HIGH);
            delay(tempo);
        }
    }

    void printBufferLog(eeCircRecord eBuffer)
    {
#ifdef DEBUGMIO
        // str[100];  // Assicurati che la dimensione sia abbastanza grande per contenere la stringa formattata
        sprintf(str, "eBuffer - wipeFlag: 0x%02X, flag: 0x%02X, pageAddress: %u, recordCount: %u",
                eBuffer.wipeFlag, eBuffer.flag, eBuffer.pageAddress, eBuffer.recordCount);
        Serial.println(str);                // Stampa sulla seriale
        wifiK::transmitMQTT(logTopic, str); // Pubblica su MQTT

#endif
    }
    void printCurmetLog(eeCircRecord eBuffer)
    {

        // str[100];  // Assicurati che la dimensione sia abbastanza grande per contenere la stringa formattata
        sprintf(str, "Curmet - wipeFlag: 0x%02X, flag: 0x%02X, pageAddress: %u, recordCount: %u",
                eBuffer.wipeFlag, eBuffer.flag, eBuffer.pageAddress, eBuffer.recordCount);

        wifiK::transmitMQTT(logMeteoTopic, str); // Pubblica su MQTT
#ifdef DEBUGMIO
        Serial.println(str); // Stampa sulla seriale

        delay(10);


#endif
    }

    void printMeteoData(meteoData *mData)
    {
#ifdef DEBUGMIO
        // Definire un buffer adeguato per la stringa
        constexpr size_t BUFFER_SIZE = 256; // Dimensione del buffer
        char str[BUFFER_SIZE];              // Buffer locale

        // Usare snprintf per evitare overflow
        int bytesWritten = snprintf(str, BUFFER_SIZE,
                                    "MeteoData - humidityBMP: %.2f, temperatureBMP: %.2f, externalPressure: %.1f, battery: %u, counter: %u, checksum: %u",
                                    mData->humidityBMP,
                                    mData->temperatureBMP,
                                    mData->externalPressure,
                                    mData->battery,
                                    mData->counter,
                                    mData->checksum);

        // Controllo se snprintf ha avuto successo
        if (bytesWritten >= 0 && static_cast<size_t>(bytesWritten) < BUFFER_SIZE)
        {
            Serial.println(str); // Stampa sulla seriale
        }
        else
        {
            // In caso di errore, loggare l'errore
            Serial.print("Errore: buffer troppo piccolo per il messaggio. Bytes written: ");
            Serial.println(bytesWritten);
        }
#endif
    }

    void printHexBuffer(const uint8_t *buffer, size_t length)
    {
#ifdef DEBUGMIO

        for (size_t i = 0; i < length; i++)
        {
            Serial.print("0x");
            if (buffer[i] < 0x10)
            {
                Serial.print("0"); // Aggiunge uno zero per valori sotto 0x10
            }
            Serial.print(buffer[i], HEX); // Stampa il valore in esadecimale
            Serial.print(" ");            // Separa i valori con uno spazio
            delay(1);
        }
        Serial.println(); // Va a capo alla fine del buffer
#endif
    }

    void handleProgramError(ErrorState &error)
    {
#ifdef DEBUGMIO

        switch (error)
        {
        case BME280_CONNECTION_FAILED:
            DEBUG_PRINT("Errore nella connessione con la sonda BME280.");
            break;
        case BME280_DATA_ERROR:
            DEBUG_PRINT("Errore nei dati letti dalla sonda BME280.");
            break;
        case ATTINY_BYTE_NUM_ERROR:
            DEBUG_PRINT("Errore nel numero di byte ricevuti dall'Attiny.");
            break;
        case ATTINY_DATA_ERROR:
            DEBUG_PRINT("Errore nei dati ricevuti dall'Attiny.");
            break;
        case EEPROM_ERROR:
            DEBUG_PRINT("Errore nell'accesso o scrittura sull'EEPROM.");
            break;
        case WIFI_CONNECTION_ERROR:
            DEBUG_PRINT("Errore nella connessione WIFI.");
            break;
        case MQTT_CONNECTION_ERROR:
            DEBUG_PRINT("Errore nella connessione MQTT.");
            break;
        case NO_ERROR:
            DEBUG_PRINT("Nessun errore, tutto OK.");
            break;
        default:
            DEBUG_PRINT("Errore sconosciuto.");
            break;
        }
#else

        blinkLed(static_cast<uint8_t>(error), true);

        // Aggiungi azioni di recovery, log o notifiche qui

#endif
    }

}