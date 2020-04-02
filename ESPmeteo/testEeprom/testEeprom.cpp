#include "testEeprom.h"
void checkI2C()
{
  byte error, address;
  int nDevices;
  Serial.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
 
    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");
 
      nDevices++;
    }
    else if (error==4)
    {
      Serial.print("Unknown error at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done");
 
  delay(5000);           // wait 5 seconds for next scan
}
void checkEEPROM(){
	uint8_t testArray [5] = { 16, 2, 77, 40, 234 }; 
    uint16_t testPos [5] = { 2, 20, 89, 32, 255 };
	for (size_t i = 0; i < 5; i++)
	{
		writeEEPROM(testPos[i],testArray[i]);
	}
	delay(100);
	for (size_t i = 0; i < 5; i++)
	{
		uint8_t myval= readEEPROM(testPos[i]);
		if(myval==testArray[i]){
			Serial.println("pos " + String(i) + String(" ok!"));
		}else{
			Serial.println("pos " + String(i) + String(" FAIL!"));
		}
		delay(250);
	}
}

void vediamoDati(){
  byte ris;
  met.humidityBMP = 6.8;
	met.temperatureBMP = 2.4;
	met.externalPressure = 1005.25;
  met.battery=1234;
  for (uint16_t i = 0; i < 6; i++)
  {
    met.humidityBMP+= 10;
	  met.temperatureBMP += 10;
	  met.externalPressure += 10;
    met.battery+=10;
    delay(20);
    ris=writeStructEEPROM(32*i);
    Serial.println("scrittura " + String(ris));
    delay(250);
  }
  for (uint16_t i = 0; i < 6; i++)
  {
    ris= readStructEEPROM(32*i);
    Serial.println("lettura " + String(ris));
    Serial.println(met.humidityBMP);
    Serial.println(met.temperatureBMP);
    Serial.println(met.externalPressure);
    Serial.println(met.battery);
    delay(250);
  }
  
  

}
void setup()
{ 
  wifi_set_sleep_type(LIGHT_SLEEP_T);
	WiFi.mode(WIFI_OFF);
	WiFi.forceSleepBegin();
	delay(100);
	Serial.begin(9600);
	DEBUG_PRINT("Booting!");
	delay(3000);
	Wire.begin(default_sda_pin, default_scl_pin);
	checkI2C();
  delay(50);
  checkEEPROM();
  delay(50);
  vediamoDati();
}
void loop(){
  delay(5000);
  vediamoDati();
  Serial.println();
}