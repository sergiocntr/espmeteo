#include "debugutils.h"
#ifndef EEPROMERROR_H
#define EEPROMERROR_H

enum class EEPROMError {
    SUCCESS,
    FATAL_ERROR,
    OUT_OF_RANGE,
    EE_FULL,
    I2C_TRANSMISSION_FAILED
};

#ifdef DEBUGMIO
const char* EEPROMError_messages[] = {
    "Operazione completata con successo.",
    "Errore persistente nella lettura dei dati dalla EEPROM!",
    "Indirizzo fuori dai limiti.",
    "Errore: Memoria EEPROM DATI piena!",
    "Errore di trasmissione I²C."
};
const char* EEPROMError_className = "EEPROMError";  // Nome della classe
#endif

#endif // EEPROMERROR_H
