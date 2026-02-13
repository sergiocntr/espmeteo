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
    "Transfer OK!",
    "Error retrieving circRecordAddress.",
    "Error during circular buffer initialization."
};
const char* CIRBUFFError_className = "CIRBUFFError";
#endif

#endif // CIRBUFFERROR_H
