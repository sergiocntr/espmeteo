// debugutils.h
#pragma once
#include <Arduino.h>
// #define DEBUGMIO
#if defined DEBUGMIO
// #define __FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)
#define DEBUG_PRINT(...) \
    Serial.print(__LINE__); \
    Serial.print(" "); \
    printArgs(__VA_ARGS__)
#else
#define DEBUG_PRINT(str)
#endif

void printArgs(const char* msg) {
    Serial.println(msg);
}

template<typename T>
void printArgs(T value) {
    Serial.println(value);
}

template<typename T, typename... Args>
void printArgs(const char* msg, T value, Args... args) {
    Serial.print(msg);
    printArgs(value, args...);
}