#include "debugutils.h"
#ifndef WIFIERROR_H
#define WIFIERROR_H

enum class WIFIError {
    SUCCESS,
    CONNECTION_FAILED
};

#ifdef DEBUGMIO
const char* WIFIError_messages[] = {
    "Connessione WIFI ok.",
    "Connessione WIFI fallita."
};
const char* WIFIError_className = "WIFIError";  // Nome della classe
#endif

#endif // WIFIERROR_H
