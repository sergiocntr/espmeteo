#ifndef cconfig_h
#define cconfig_h
//ESP-01 SDA - SCL pin
static int default_sda_pin = 0;
static int default_scl_pin = 2;
//WIFI stuff
const char* ssid     = "TIM-23836387";
const char* password = "51vEBuMvmALxNQHVIHQKkn52";
const char* webpass ="admin";
#include <ESP8266WiFi.h>
IPAddress ip(192, 168, 1, 211); //Node static IP
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
// data web host Settings
#define host "www.developteamgold.altervista.org"
const int httpPort = 80;
//voltage stuff
uint16_t voltage = 0;  //voltage get from attiny
uint8_t dati[2];       // attiny low and high voltage byte
float humidityDHT22,temperatureDHT22,Humidex,dp,p0;
//I2C eeprom stuff
const int SLAVE_ADDRESS = 0X50; //classic I2C EEPROM address
const uint16_t nValuesAddr = 0x0FFF; //address on I2C EEPROM ,we store there how many reecords we have
//meteo data object + battery
struct meteoData{
	float humidityDHT22;
	float temperatureDHT22;
	float externalPressure;
  int battery;
};
typedef struct meteoData MeteoData;
MeteoData met, retmet;

const char* mqtt_server = "192.168.1.100";
const char* sensorsTopic = "/casa/esterno/sensori";
#endif
