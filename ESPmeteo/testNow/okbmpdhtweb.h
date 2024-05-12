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
float truncate(float val) 
{
    int x =(int)(val * 100);
    float y=0.00;
    y= (float)x/100;
    return y;
   
}

bool bm(meteoData& currmet)
{
  float temp(NAN), hum(NAN), pres(NAN);
  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_hPa);
  uint32_t mytime=millis();
  while(!bme.begin())
  {
    if (millis()-mytime > 3000) {
      #ifdef DEBUGMIO
      char str[80];
			sprintf(str, "Timeout  dati meteo .");
      Serial.println(str);
      //publish(logTopic,str);
      //smartDelay(100);
			#endif
      return 0;
    }
    delay(400);
  }
  //delay(400);
  bme.read(pres, temp, hum,tempUnit,presUnit);
  delay(10);
  // pres = truncate(pres);
  // temp = truncate(temp);
  // hum = truncate(hum);
  // delay(10);
  currmet.humidityBMP = hum;
	currmet.temperatureBMP = temp;
	currmet.externalPressure = pres;
  delay(10);
  #ifdef DEBUGMIO
  char str[80];
			sprintf(str, "Dati meteo ok.");
      Serial.println(str);
      //publish(logTopic,str);
      //smartDelay(100);
			#endif
  return 1;
}
#endif
