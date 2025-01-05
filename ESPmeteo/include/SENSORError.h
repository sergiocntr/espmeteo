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
    "BME280 ok.",
    "Il sensore BME280 non risponde.",
    "Dati non validi letti dal sensore BME280."
};
const char* SENSORError_className = "SENSORError";  // Nome della classe
#endif

#endif // SENSORERROR_H
