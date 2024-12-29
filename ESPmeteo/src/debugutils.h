// debugutils.h
#pragma once
#include <Arduino.h>
// #define DEBUGMIO
#if defined DEBUGMIO
// #define __FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)
#define DEBUG_PRINT(...)   \
   Serial.print(__LINE__); \
   Serial.print(" ");      \
   Serial.println(__VA_ARGS__);
#else
#define DEBUG_PRINT(str)
#endif

// void testDebugPrint()
// {
//     int errorAddress = 42;
//     int errorCode = -1;
//     const char* errorMsg = "Errore di scrittura";

//     // Usando DEBUG_PRINT con più parametri
//     DEBUG_PRINT("Messaggio:", errorMsg, "| Indirizzo:", errorAddress, "| Codice errore:", errorCode);
// }