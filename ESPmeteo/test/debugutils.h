// debugutils.h
#pragma once
#define DEBUG_MIO
#ifdef DEBUG_MIO
#define __FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)
#define DEBUG_PRINT(str)    \
     Serial.print(__FILENAME__);     \
     Serial.print(' ');      \
     Serial.print(__LINE__);     \
     Serial.print(' ');      \
     Serial.println(str);
#else 
    #define DEBUG_PRINT(str)
#endif