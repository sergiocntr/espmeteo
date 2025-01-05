#include "debugutils.h"
#ifndef MQTTERROR_H
#define MQTTERROR_H

enum class MQTTError {
    SUCCESS,
    CONNECTION_FAILED,
    PUBLISH_FAILED,
    SUBSCRIBE_FAILED,
    ACK_FAILED,
    ACK_OK,
    END_OK
};

#ifdef DEBUGMIO
const char* MQTTError_messages[] = {
    "Connessione MQTT OK!",
    "Connessione MQTT fallita.",
    "Pubblicazione del messaggio fallita.",
    "Iscrizione al topic fallita.",
    "ACK non ricevuto ento il timeout",
    "Dati parziali inviati e confermati con successo!",
    "Dati totali inviati e confermati con successo!"
};
const char* MQTTError_className = "MQTTError";  // Nome della classe
#endif

#endif // MQTTERROR_H
