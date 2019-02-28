#ifndef okbmpdhtweb_h
#define okbmpdhtweb_h
#include <BME280I2C.h>
#include <Wire.h>

#define SERIAL_BAUD 115200

BME280I2C bme;    // Default : forced mode, standby time = 1000 ms
                  // Oversampling = pressure ×1, temperature ×1, humidity ×1, filter off,
void bm(float& somtemp,float& somhum,float& sompres)
{
  bme.begin();

  float temp(NAN), hum(NAN), pres(NAN);

  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_hPa);


  uint16 mytemp=0,mypres=0,myhum=0;
  for (int mis = 0; mis < 5; mis++) {
    delay(1000);
    bme.read(pres, temp, hum,tempUnit,presUnit);
    somtemp+=temp;
    sompres+=pres;
    somhum+=hum;
  }
  somtemp/=5;
  sompres/=5;
  somhum/=5;
  //mytemp=somtemp*20;
  //mypres=sompres*20;
  //myhum=somhum*20;
  //somtemp=mytemp/100;
  //sompres=mypres/100;
  //somhum=myhum/100;

}
#endif
