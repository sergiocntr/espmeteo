#ifndef DEBUGUTILS_H
#define DEBUGUTILS_H
#include <Arduino.h>
//*****ENABLE FOR DEBUG
//#define DEBUGMIO
//#define DEBUGMQTT
  #ifdef DEBUGMIO
  #define __FILE_NAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

  #define DEBUG_PRINT(str)    \
     Serial.print(millis());     \
     Serial.print(": ");    \
     Serial.print(__PRETTY_FUNCTION__); \
     Serial.print(' ');      \
     Serial.print(__FILE_NAME__);     \
     Serial.print(':');      \
     Serial.print(__LINE__);     \
     Serial.print(' ');      \
     Serial.println(str); \
     Serial.flush();  \

  #else
  #define DEBUG_PRINT(str)
  #endif
  #ifdef DEBUGMQTT
  #define DEBUG_MQTT(str)    \
     printMqttLog(String(str));
     //printMqttLog(str);

  #else
  #define DEBUG_MQTT(str)
  #endif
#endif
