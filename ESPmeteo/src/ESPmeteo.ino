//sergiocntr@gmail.com
#include "DHT.h"
#include <Wire.h>
#include <SPI.h>
//#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <ESP8266WiFi.h>
#include <I2C_Anything.h> //http://www.gammon.com.au/forum/?id=10896&reply=8#reply8
//ESP-01 SDA - SCL pin
static int default_sda_pin = 0;
static int default_scl_pin = 2;
//WIFI stuff
WiFiClient c;
IPAddress ip(192, 168, 1, 211); //Node static IP
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
// data web host Settings
#define host "www.developteamgold.altervista.org"
const int httpPort = 80;
//voltage stuff
uint16_t voltage = 0;  //voltage get from attiny
uint8_t dati[2];       // attiny low and high voltage byte
//BMP stuff
Adafruit_BMP280 bmp; // I2C
double p0;
//DHT22 stuff
#define DHTPIN 3  //GPIO1 (Rx) what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);
float humidityDHT22,temperatureDHT22,Humidex,dp;
//I2C eeprom stuff
const int SLAVE_ADDRESS = 0X50; //classic I2C EEPROM address
const uint16_t nValuesAddr = 0x0FFF; //address on I2C EEPROM ,we store there how many reecords we have
//meteo data object + battery
struct meteoData{
	float humidityDHT22;
	float temperatureDHT22;
	double externalPressure;
  int battery;
};
typedef struct meteoData MeteoData;
MeteoData met, retmet;
void setup()
{
	Serial.begin(9600);
	delay(500); 									// do tempo a Attiny di leggere la tensione
	Serial.println("OK");
	uint8_t check = connLAN(); 		//check == 1 -> connected to local WIFI
	Wire.begin(default_sda_pin, default_scl_pin);
	uint8_t value = readEEPROM(nValuesAddr); //have we records stored in I2C ?
	Serial.println("records: " + value);
	if(value==255) { //only for debug reason
		value = 0;
		writeEEPROM(nValuesAddr,value);
	}
	if(check == 1 && value > 0) sendData(value); // if WIFI available and records stored, send them to server first

	requestSensorsValues();

	if(check == 1){	//  if local WIFI connection OK send data without save them to I2C Eeprom
		printWEB(true); // send data to server (true = get time from web server (live record))
	}
  else{		//local WIFI connection KO
		uint16_t availAddress = 32 * value;	//MeteoData are only 24 bytes  but write page on eeprom need to be 32 bytes long
    //compile struct object with current data
    met.battery = voltage;
    met.humidityDHT22 = humidityDHT22;
    met.temperatureDHT22 = temperatureDHT22;
    met.externalPressure = p0;
		writeStructEEPROM(availAddress);	//write struct on I2C eeprom
		value++; //add record's nr
		writeEEPROM(nValuesAddr,value); //update storage records nr on I2C eeprom
  }
}
void loop(){
  Serial.println("Please send me to sleep...");
  delay(500);
	Wire.begin(default_sda_pin, default_scl_pin);
  Wire.beginTransmission (2);
  Wire.write (20);
  Wire.endTransmission(true);

}
//data
void requestSensorsValues(){
	//while (voltage < 3500 | voltage > 5000){			//sanity check about voltage value
    //Wire.begin(default_sda_pin, default_scl_pin);		//better way welcome!
    for (int i=0; i <= 2; i++){
      Wire.requestFrom(2, 2);    // request 2 bytes from slave device #2---- Wire.requestFrom (SLAVE_ADDRESS, responseSize);
      dati[i] = Wire.read();    // receive a byte as character
    }
    voltage = (dati[1]<<8) | dati[0];
  //}
  Serial.println("voltage : " + String(voltage));
	sensor_init();
  delay(50);
  bm();									// read BMP080 values
  Serial.println("pressure : " + String(p0));
  dh();									//read DHT22 values
  Serial.println("temp : " + String(temperatureDHT22));

}
//WIFI
uint8_t connLAN()
{
	char* ssid     = "";
	char* password = "";
	passWifi(ssid,password);
	uint8_t check=0;
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.config(ip, gateway, subnet); // Set static IP (2,7s) or 8.6s with DHCP  + 2s on battery
  for (int i=0; i <= 5; i++){
    if(WiFi.status() == WL_CONNECTED)
      check = 1;
    delay(700);
  }
	if (check == 0) WiFi.mode(WIFI_OFF); //energy saving mode if local WIFI isn't connected
  return check;
}
void printWEB(bool timeAvailable) //timeAvailable -> live mesaures
{
  if (c.connect(host, httpPort))
  {
		String webpass     = "";
		passWeb(webpass);
		double gamma = log(humidityDHT22 / 100) + ((17.62 * temperatureDHT22) / (243.5 + temperatureDHT22));
	  dp = 243.5 * gamma / (17.62 - gamma);
	  double Humidex = temperatureDHT22 + (5 * ((6.112 * pow( 10, 7.5 * temperatureDHT22/(237.7 + temperatureDHT22))*humidityDHT22/100) - 10))/9;
		Serial.println("connected");
    // Make a HTTP request:
    String s =String("GET /meteofeletto/swpi_logger.php?temp_out=" + String(temperatureDHT22) +
    +"&&pwd=" + webpass +
    +"&&hum_out=" + String(humidityDHT22) +
    +"&&rel_pressure=" + String(p0) +
    +"&&dwew=" + String(dp) +
    +"&&humidex=" + String(Humidex) +
    +"&&voltage=" + String(voltage) +
		+"&&time=" + String(timeAvailable) +
    + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
    Serial.println(s);
    c.println(s);
  }
}

//I2C EEPROM
void sendData(uint8_t nrRecords){ // send stored I2C eeprom meteo data to web server
	for (int i = 0 ; i <= (nrRecords - 1); i++){
		readStructEEPROM(32 * i); // read I2C eeprom
		voltage = met.battery;
    humidityDHT22 = met.humidityDHT22 ;
    temperatureDHT22 = met.temperatureDHT22 ;
    p0 = met.externalPressure ;
		printWEB(false);// false add time from last web server record (recorded record)
	}
	writeEEPROM(nValuesAddr,0); //reset storage records nr on I2C eeprom
}
byte writeStructEEPROM(unsigned int addr)
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
byte readStructEEPROM(unsigned int addr)
{
  byte err;
	Wire.beginTransmission (SLAVE_ADDRESS);
  Wire.write ((byte) (addr >> 8));    // high order byte
  Wire.write ((byte) (addr & 0xFF));  // low-order byte
  err = Wire.endTransmission ();
	// initiate blocking read into internal buffer
  Wire.requestFrom (SLAVE_ADDRESS, sizeof(met));

  I2C_readAnything (met);

  return err;
}
void writeEEPROM(uint16_t eeaddress, uint8_t data )
{
  Wire.beginTransmission(SLAVE_ADDRESS);
  Wire.write((int)(eeaddress >> 8));   // MSB
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.write(data);
  Wire.endTransmission();

  delay(6);
}
uint8_t readEEPROM(uint16_t eeaddress )
{
  uint8_t rdata = 0xFF;

  Wire.beginTransmission(SLAVE_ADDRESS);
  Wire.write((int)(eeaddress >> 8));   // MSB
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.endTransmission();

  Wire.requestFrom(SLAVE_ADDRESS,1);

  if (Wire.available()) rdata = Wire.read();

  return rdata;
}
