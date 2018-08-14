#ifndef DEBUGUTILS_H
#define DEBUGUTILS_H
#include <Arduino.h>
//*****ENABLE FOR DEBUG
//#define DEBUGMIO
//#define DEBUGMQTT
  #ifdef DEBUGMIO
  #define DEBUG_PRINT(str)    \
     Serial.print(millis());     \
     Serial.print(": ");    \
     Serial.print(__PRETTY_FUNCTION__); \
     Serial.print(' ');      \
     Serial.print(__FILE__);     \
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
