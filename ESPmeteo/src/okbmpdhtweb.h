#ifndef okbmpdhtweb_h
#define okbmpdhtweb_h
#include "cxonfig.h"
#include <BME280I2C.h>
#include <Wire.h>
BME280I2C::Settings settings(
   BME280::OSR_X1,
   BME280::OSR_X1,
   BME280::OSR_X1,
   BME280::Mode_Forced,
   BME280::StandbyTime_1000ms,
   BME280::Filter_Off,
   BME280::SpiEnable_False
   //BME280I2C::I2CAddr_0x76 // I2C address. I2C specific.
);
BME280I2C bme(settings);
//BME280I2C bme;
uint8_t bm(meteoData& met)
{
  float temp(NAN), hum(NAN), pres(NAN);
  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_hPa);
  unsigned long mytime=millis();
  while(!bme.begin())
  {
    if (millis()-mytime > 2000) {
      return 1;
    }
    delay(400);
  }
  bme.read(pres, temp, hum,tempUnit,presUnit);
  met.humidityBMP = hum;
	met.temperatureBMP = temp;
	met.externalPressure = pres;
  return 0;
}
#endif
