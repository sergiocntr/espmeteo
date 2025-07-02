// m_wifi.h
#pragma once
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
// #include <ESP8266Ping.h>
#include "debugutils.h"
#include "topic.h"
#include "password.h"
#include "myIP.h"
#include "impostazioni.h"
WiFiClient espClient;
PubSubClient mqttClient(espClient);
const char *remote_host = "www.google.com";
namespace wifiK
{

    bool enableTrasmit = true;
    bool isMQTTConnected()
    {
        return mqttClient.connected();
    }

    void smartDelay(unsigned long ms)
    {

        unsigned long start = millis();
        do
        {

            mqttClient.loop();
            delay(2);
        } while (millis() - start < ms);
    }

    volatile AckState ackReceived = NO_ACK; // Protegge l'accesso concorrente

    void callback(char *topic, byte *payload, unsigned int length)
    {
        if (strcmp(topic, ackMeteoTopic) == 0)
        {
            String ackMessage = "";
            for (unsigned int i = 0; i < length; i++)
            {
                ackMessage += (char)payload[i];
            }
            //Serial.println("ACK ricevuto: " + ackMessage);
            if (strcmp(ackMessage.c_str(), "ack") == 0)
            {
                ackReceived = ACK;
            }
            else if (strcmp(ackMessage.c_str(), "end") == 0)
            {
                ackReceived = END;
            }
            else if (strcmp(ackMessage.c_str(), "failed") == 0)
            {
                ackReceived = FAILED;
            }
            else if (strcmp(ackMessage.c_str(), "error") == 0)
            {
                ackReceived = ERROR;
            }
        }
    }

    void waitForAck(AckState &ackMsg)
    {
        unsigned long startMillis = millis();
        while (millis() - startMillis < 1500)
        {
            smartDelay(10); // Assicura che PubSubClient.loop() sia chiamato durante smartDelay
            if (ackReceived != NO_ACK)
            {
                ackMsg = ackReceived;
                ackReceived = NO_ACK; // Consuma lo stato dell'ACK
                return;
            }
        }
        ackMsg = NO_ACK;
    }

    void connectMQTT(ErrorState &error)
    {
        mqttClient.setServer(mqtt_server, mqtt_port);
        mqttClient.setCallback(callback);
        String clientId = String(marinerId);
        clientId += String(random(0xffff), HEX);
        delay(100);
        mqttClient.connect(clientId.c_str(), mqttUser, mqttPass);
        uint32_t wifi_initiate = millis();
        smartDelay(100);
        while (!mqttClient.connected())
        {
            if ((millis() - wifi_initiate) > 5000L)
            {
                error = MQTT_CONNECTION_ERROR;

                return;
            }
            delay(200);
        }
        delay(100);
        mqttClient.subscribe(ackMeteoTopic);

        error = NO_ERROR;
    }
    void setWifiOff()
    {
        if (mqttClient.connected())
        {
            mqttClient.disconnect();
            smartDelay(100);
        }
        // Metti il WiFi in sleep leggero per ridurre il consumo energetico
        WiFi.setSleep(true);
        WiFi.disconnect(true); // Disconnetti dalla rete e cancella le credenziali
        delay(100);            // Attendi un po' per completare la disconnessione

        // Forza lo sleep e spegni il modulo WiFi
        WiFi.forceSleepBegin();
        delay(50);           // Attendi che lo sleep venga effettivamente attivato
        WiFi.mode(WIFI_OFF); // Spegni il WiFi
    }
    void connectWifi(ErrorState &error)
    {
        // setIP(marinerUan,marinerId);
        WiFi.hostname("espMeteo"); // DHCP Hostname (useful for finding device for static lease)
        WiFi.mode(WIFI_STA);
        WiFi.forceSleepWake();
        delay(10);
        WiFi.config(marinerUan, gateway, subnet, dns1); // Set static IP (2,7s) or 8.6s with DHCP  + 2s on battery
        delay(10);
        WiFi.begin(ssid, password);
        delay(10);
        // Serial.println("wifi");
        uint32_t wifi_initiate = millis();
        while (WiFi.status() != WL_CONNECTED)
        {
            if ((millis() - wifi_initiate) > 8000L)
            {
                error = WIFI_CONNECTION_ERROR;
                return;
            }
            delay(500);
        }
        delay(100);
        // if (!Ping.ping(remote_host))
        // {
        // 	return false;
        // }
        error = NO_ERROR;
    }

    bool transmitMQTT(const char *_topic, char *_message)
    {
        if (!enableTrasmit)
        {
            Serial.println("transmission disabled!.");
            return false;
        }

        if (isMQTTConnected())
        {
            //smartDelay(10);
            return mqttClient.publish(_topic, _message);
        }
        else
        {
            Serial.println("Errore: MQTT non collegato.");
            return false;
        }
    }

    bool transmitMQTT(const char *_topic, const uint8_t *_message, size_t length)
    {
        if (!enableTrasmit)
        {
            Serial.println("transmission disabled!.");
            return false ;
        }

        
        if (isMQTTConnected())
        {
            smartDelay(100);
            return mqttClient.publish(_topic, _message, length);
        }
        else
        {
            Serial.println("Errore:  MQTT non collegato.");
            return false;
        }
    }

    bool sendDataViaMQTT(uint8_t *rawBuffer, size_t bufferLength)
    {
        if (!enableTrasmit)
        {
            Serial.println(" sendDataViaMQTT : transmission disabled!.");
            return true;
        }
        
        uint8_t nrTry = 0;
        bool sent = false;
        while (!sent && nrTry < 2)
        {
            sent = transmitMQTT(meteoDataTopic, rawBuffer, bufferLength);
            if (!sent)
            {
                nrTry++;
                smartDelay(500);
                Serial.printf("Tentativo %d fallito, riprovo...\n", nrTry);
                ERROR_PRINT(MQTTError_className, MQTTError, MQTTError::PUBLISH_FAILED);
            }
        }
        return sent;
    }

    bool sendWithRetry(uint8_t *rawBuffer, size_t bufferSize, Eeprom_job &ejob)
    {
        if (!enableTrasmit)
        {
            Serial.println(" sendWithRetry : transmission disabled!.");
            return true ;
        }

        
        
        AckState ackStatus = NO_ACK;
        uint8_t retries = 1; // Numero massimo di ritrasmissioni

        while (retries >= 0)
        {
            // Invia il messaggio al server
            bool sent = sendDataViaMQTT(rawBuffer, bufferSize);

            if (sent)
            {
                waitForAck(ackStatus); // Aspetta l'ACK

                // Gestione dell'ACK ricevuto
                if (ackStatus == ACK)
                {
                    ejob = NO_JOB;
                    //Serial.println("Dati parziali confermati con successo!");
                    return true; // Successo
                }
                else if (ackStatus == FAILED)
                {
                    Serial.println("Errore nel server: ritrasmissione del messaggio...");
                    retries--; // Riduce il numero di ritrasmissioni disponibili
                    continue;  // Ritrasmette il messaggio
                }
                else if (ackStatus == ERROR)
                {
                    ejob = ERASE_EEPROM;
                    Serial.println("Errore critico ricevuto dal server. Reset dei messaggi...");
                    // initializeCircularRecord(); // Resetta i messaggi partendo da zero
                    return false; // Errore critico
                }
                else if (ackStatus == END)
                {
                    ejob = UPDATE_EEPROM;
                    Serial.println("Dati inviati e confermati con successo!");
                    // forwardCircularRecord(); // Gestisce il caso END
                    return true;
                }
                else
                {
                    ERROR_PRINT(MQTTError_className, MQTTError, MQTTError::ACK_FAILED);
                    Serial.println("ACK non valido o non ricevuto. se il record non era aggiunto verra aggiunto adesso");
                    ejob = NO_JOB; // niente da fare
                    return false;
                }
            }
            else
            {
                Serial.println("Errore nella trasmissione. Ritento...");
                smartDelay(500); // Aspetta un po' prima di ritrasmettere
                retries--;       // Riduce il numero di tentativi
            }
        }

        Serial.println("Tutti i tentativi di trasmissione falliti.");
        ejob = NO_JOB; // niente da fare
        return false;  // La trasmissione fallisce
    }

    // bool transmitHTTP(char *meteoPayload)
    // {
    //     HTTPClient http;
    //     smartDelay(10);
    //     http.begin(espClient, post_serverJSON);
    //     smartDelay(100);
    //     int httpResponseCode = http.PUT(meteoPayload);
    //     smartDelay(100);

    //     String payload = ""; // Variabile per memorizzare la risposta del server
    //     if (httpResponseCode > 0)
    //     {
    //         // La richiesta è stata eseguita con successo, otteniamo la risposta
    //         payload = http.getString();
    //         DEBUG_PRINT("HTTP Response code: ");
    //         DEBUG_PRINT(httpResponseCode);
    //         DEBUG_PRINT("Server response: ");
    //         DEBUG_PRINT(payload);
    //     }
    //     else
    //     {
    //         // Errore durante la richiesta
    //         DEBUG_PRINT("Errore HTTP: ");
    //         DEBUG_PRINT(httpResponseCode);
    //     }

    //     http.end(); // Free resources
    //     delay(100);

    //     // Valutazione del risultato
    //     if (httpResponseCode == 200)
    //         return true;

    //     return false;
    // }
}