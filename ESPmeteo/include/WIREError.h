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
    "Transfer OK!",
    "Requested byte count mismatch.",
    "Incomplete data received."
};
const char* WIREError_className = "WIREError";
#endif

#endif // WIREERROR_H
