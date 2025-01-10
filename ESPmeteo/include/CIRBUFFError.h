#include "debugutils.h"
#ifndef CIRBUFFERROR_H
#define CIRBUFFERROR_H

enum class CIRBUFFError {
    SUCCESS,
    CIRCREC_NOT_FOUND,
    CIRCREC_NOT_BUILD
};

#ifdef DEBUGMIO
const char* CIRBUFFError_messages[] = {
    "Transfer ok!.",
    "Errore nel recuperare circRecordAddress.",
    "Errore durante l'inizializzazione del buffer circolare."
};
const char* CIRBUFFError_className = "CIRBUFF";  // Nome della classe
#endif

#endif // CIRBUFFERROR_H
