#ifndef Checkvoltage_h
#define Checkvoltage_h
#include "Arduino.h"
class Checkvoltage
{
  public:
    Checkvoltage();
    void readVcc();
    void begin();
    uint16_t  volt;
    uint8_t dati[2];
  private:
    int _pin;
    long batteryAlertThreshhold;
};
#endif
