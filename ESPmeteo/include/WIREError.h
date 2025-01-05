#include "debugutils.h"
#ifndef WIREERROR_H
#define WIREERROR_H

enum class WIREError {
    SUCCESS,
    BYTE_NUM_ERROR,
    DATA_ERROR
};

#ifdef DEBUGMIO
const char* WIREError_messages[] = {
    "Transfer ok!.",
    "Numero di byte richiesti non corrisponde.",
    "Dati incompleti ricevuti"
};
const char* WIREError_className = "WIREError";  // Nome della classe
#endif

#endif // WIREERROR_H
