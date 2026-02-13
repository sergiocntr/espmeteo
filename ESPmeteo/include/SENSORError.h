#include "debugutils.h"

#ifndef SENSORERROR_H
#define SENSORERROR_H

enum class SENSORError {
    BME280_SUCCESS,
    BME280_CONNECTION_FAILED,
    BME280_DATA_ERROR
};

#ifdef DEBUGMIO
const char* SENSORError_messages[] = {
    "BME280 OK.",
    "BME280 sensor not responding.",
    "Invalid data read from BME280 sensor."
};
const char* SENSORError_className = "SENSORError";
#endif

#endif // SENSORERROR_H
