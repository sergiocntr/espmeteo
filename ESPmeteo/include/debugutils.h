#pragma once
#include <Arduino.h>

// Abilitazione debug
//#define DEBUGMIO

#if defined DEBUGMIO
// Macro per debug generico
#define DEBUG_PRINT(...)   \
    Serial.print(__LINE__); \
    Serial.print(" ");      \
    Serial.println(__VA_ARGS__);

// Macro per errori specifici
#define ERROR_PRINT(className, errorEnum, errorCode)                      \
    do {                                                                   \
        Serial.print(__LINE__);                                            \
        Serial.print(" Log di ");                                        \
        Serial.print(className);                                           \
        Serial.print(" - ");                                                \
        Serial.print(static_cast<int>(errorCode));                         \
        Serial.print(": ");                                                \
        Serial.println(errorEnum##_messages[static_cast<int>(errorCode)]); \
    } while (0)

#else
// Disabilitazione debug
#define DEBUG_PRINT(...)
#define ERROR_PRINT(className, errorEnum, errorCode)
#endif
