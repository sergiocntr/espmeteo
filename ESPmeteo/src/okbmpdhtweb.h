#ifndef okbmpdhtweb_h
#define okbmpdhtweb_h
#include <BME280I2C.h>
#include <Wire.h>

//#define SERIAL_BAUD 115200

//BME280I2C bme;    // Default : forced mode, standby time = 1000 ms
                  // Oversampling = pressure ×1, temperature ×1, humidity ×1, filter off,
BME280I2C::Settings settings(
   BME280::OSR_X1,
   BME280::OSR_X1,
   BME280::OSR_X1,
   BME280::Mode_Forced,
   BME280::StandbyTime_1000ms,
   BME280::Filter_Off,
   BME280::SpiEnable_False,
   BME280I2C::I2CAddr_0x76 // I2C address. I2C specific.

);
//BME280I2C bme(settings);
BME280I2C bme;
uint8_t bm(float& somtemp,float& somhum,float& sompres)
{
  //float temp(NAN), hum(NAN), pres(NAN);

  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_hPa);
  unsigned long mytime=millis();
  while(!bme.begin())
  {
    //Serial.println("Could not find BME280 sensor!");
    if (millis()-mytime > 2000) {
      return 1;
    }
    delay(400);
  }

  //uint16 mytemp=0,mypres=0,myhum=0;
  //for (char mis = 0; mis < 4; mis++) {
  //  bme.read(pres, temp, hum,tempUnit,presUnit);
  //  somtemp+=temp;
  //  sompres+=pres;
  //  somhum+=hum;
  //  while(!bme.begin());
  //}
  delay(1000);
  bme.read(sompres, somtemp, somhum,tempUnit,presUnit);
  //somtemp/=4;
  //sompres/=4;
  //somhum/=4;
  //mytemp=somtemp*20;
  //mypres=sompres*20;
  //myhum=somhum*20;
  //somtemp=mytemp/100;
  //sompres=mypres/100;
  //somhum=myhum/100;
  return 0;
}
#endif
