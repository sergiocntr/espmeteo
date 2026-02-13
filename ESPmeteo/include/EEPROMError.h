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
    "Operation completed successfully.",
    "Persistent error reading data from EEPROM!",
    "Address out of range.",
    "Error: EEPROM data memory full!",
    "I2C transmission error."
};
const char* EEPROMError_className = "EEPROMError";
#endif

#endif // EEPROMERROR_H
