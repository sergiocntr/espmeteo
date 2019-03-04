#ifndef cxconfig_h
#define cxconfig_h
//ESP-01 SDA - SCL pin
static int default_sda_pin = 0;
static int default_scl_pin = 2;
const char* host = "************************";
const int httpPort = 80;
const char* post_serverJSON = "****************";
const char* post_server = "*****************************";
uint16_t voltage = 0;  //voltage get from attiny
uint8_t dati[2];       // attiny low and high voltage byte
float humidityBMP,temperatureBMP,Humidex,p0;
//I2C eeprom stuff
const int SLAVE_ADDRESS = 0X50; //classic I2C EEPROM address
const uint16_t nValuesAddr = 0x0FFF; //address on I2C EEPROM ,we store there how many reecords we have
//meteo data object + battery
struct meteoData{
	float humidityBMP;
	float temperatureBMP;
	float externalPressure;
  int battery;
};
typedef struct meteoData MeteoData;
MeteoData met, retmet;
#endif
