#include <Wire.h> //I2C library
#include <I2C_Anything.h>

#define SERIAL_OUT Serial

struct meteoData{
	float humidityDHT22;
	float temperatureDHT22;
	double externalPressure;
  int battery;
} meteoData;
typedef struct meteoData MeteoData;
MeteoData met, retmet;
const int SLAVE_ADDRESS = 0X50;
void setup()
{
  byte b;
	delay(1000);
  Serial.begin(57600);
  delay(1000);
  SERIAL_OUT.println("\nprova memoria\n");
  Wire.begin (0,2);
  met.battery = 4300;
  met.humidityDHT22 = 75.40;
  met.temperatureDHT22 = 22.30;
  b=writeEEPROM(0X00);
	SERIAL_OUT.println("codice scrittura 0X00 (deve essere 0) " + String(b));
  b=readEEPROM(0X00);
	SERIAL_OUT.println("codice lettura 0X00 (deve essere 0) " + String(b));
  SERIAL_OUT.println("batteria " + String(retmet.battery));
  SERIAL_OUT.println("hum " + String(retmet.humidityDHT22));
  SERIAL_OUT.println("temp " + String(retmet.temperatureDHT22));
  SERIAL_OUT.println("misura " + String(sizeof(met)));
  SERIAL_OUT.println("misura ritornata " + String(sizeof(retmet)));
  b=writeEEPROM(0X24);
	SERIAL_OUT.println("codice scrittura 0X24 (deve essere 0) " + String(b));
  b=readEEPROM(0X24);
	SERIAL_OUT.println("codice lettura 0X24 (deve essere 0) " + String(b));
	SERIAL_OUT.println("batteria " + String(retmet.battery));
  SERIAL_OUT.println("hum " + String(retmet.humidityDHT22));
  SERIAL_OUT.println("temp " + String(retmet.temperatureDHT22));
  SERIAL_OUT.println("misura " + String(sizeof(met)));
  SERIAL_OUT.println("misura ritornata " + String(sizeof(retmet)));
  //writeEEPROM(0X00,met);
  b=readEEPROM(0X00);
	SERIAL_OUT.println("codice lettura 0X00 (deve essere 0) " + String(b));
	SERIAL_OUT.println("batteria " + String(retmet.battery));
  SERIAL_OUT.println("hum " + String(retmet.humidityDHT22));
  SERIAL_OUT.println("temp " + String(retmet.temperatureDHT22));
  SERIAL_OUT.println("misura " + String(sizeof(met)));
  SERIAL_OUT.println("misura ritornata " + String(sizeof(retmet)));
}

void loop(){
  yield();
}
byte writeEEPROM(unsigned int addr)
{
	byte err;
	Wire.beginTransmission(SLAVE_ADDRESS);
	Wire.write ((byte) (addr >> 8));    // high order byte
	Wire.write ((byte) (addr & 0xFF));  // low-order byte
	I2C_writeAnything (met);
	err = Wire.endTransmission ();
	delay(6);  // needs 5ms for page write
	return err;  // cannot write to device
  }


byte readEEPROM(unsigned int addr)
{

  byte err;
	Wire.beginTransmission (SLAVE_ADDRESS);
  Wire.write ((byte) (addr >> 8));    // high order byte
  Wire.write ((byte) (addr & 0xFF));  // low-order byte
  err = Wire.endTransmission ();

  if (err != addr)
    return err;  // cannot read from device

  // initiate blocking read into internal buffer
  Wire.requestFrom (SLAVE_ADDRESS, sizeof(retmet));

  I2C_readAnything (retmet);

  return 0;  // OK
  }  // end of readEEPROM
