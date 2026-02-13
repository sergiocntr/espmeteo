#include "debugutils.h"

#ifndef WIFIERROR_H
#define WIFIERROR_H

enum class WIFIError {
    SUCCESS,
    CONNECTION_FAILED
};

#ifdef DEBUGMIO
const char* WIFIError_messages[] = {
    "WiFi connection OK.",
    "WiFi connection failed."
};
const char* WIFIError_className = "WIFIError";
#endif

#endif // WIFIERROR_H
