/*
questo e' il codice che gira sull ESP meteo
aggiorna il voltaggio
spegne ESP
*/
//#include "DHT.h"
//#include <SFE_BMP180.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
//#include <EEPROM.h>
#include <I2C_Anything.h>
//ESP-01 SDA - SCL pin
static int default_sda_pin = 0;
static int default_scl_pin = 2;
//WIFI stuff
WiFiClient c;
const char* ssid     = "TIM-18232399";
const char* password = "ObXtYwlWaqnXIJjqs2NbF6qP";
IPAddress ip(192, 168, 1, 211); //Node static IP
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
// data web host Settings
#define host "www.developteamgold.altervista.org"
const int httpPort = 80;
//voltage stuff
uint16_t voltage = 0;  //voltage get from attiny
//uint8_t dati[2];        // attiny low and high voltage byte
//BMP stuff
//SFE_BMP180 pressure;
//char status;
double T,P,p0,a;
//DHT22 stuff
//#define DHTPIN 1  //GPIO1 (Tx) what digital pin we're connected to
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//DHT dht(DHTPIN, DHTTYPE);
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
#define SERIAL_OUT Serial
void setup()
{
	//begin section
	Serial.begin(9600);
	Wire.begin(default_sda_pin, default_scl_pin);
  //delay(500); 									// do tempo a Attiny di leggere la tensione
	writeEEPROM(nValuesAddr,0); //reset storage records nr on I2C eeprom

}
void loop(){
	uint8_t check = 0;//connLAN(); 		//check == 1 -> connected to local WIFI
	uint8_t nRecords = readEEPROM(nValuesAddr); //have we records stored in I2C ?
	for (int i = 0 ; i <= 6; i++){
		//if(check == 1 && nRecords > 0) sendData(nRecords); // if WIFI available and records stored, send them to server
		//data section
		requestSensorsValues();
	  if(check == 1){	//  if local WIFI connection OK send data without save to I2C

			printWEB(true); // send data to server (true = get time from web server (live record))
		}
	  else{		//local WIFI connection KO
			uint16_t availAddress = 32 * nRecords;	//MeteoData is 24 bytes long so..
	    //compile struct object with current data
	    met.battery = voltage;
	    met.humidityDHT22 = humidityDHT22;
	    met.temperatureDHT22 = temperatureDHT22;
	    met.externalPressure = p0;
			writeStructEEPROM(availAddress);	//write struct on I2C eeprom
			nRecords++; //add record's nr
			writeEEPROM(nValuesAddr,nRecords); //update storage records nr on I2C eeprom
		}
	}
	delay(3000);
	nRecords = readEEPROM(nValuesAddr); //have we records stored in I2C ?
	sendData(nRecords);
}
//data
void requestSensorsValues(){
	voltage=4500;
	p0=1000.56;
	temperatureDHT22=21.72;
	humidityDHT22=78.25;
}
void sendData(uint8_t nrRecords){ // send stored I2C eeprom meteo data to web server
	SERIAL_OUT.println("\nINVIO " + String(nrRecords) + " RECORD");
	for (int i = 0 ; i <= (nrRecords - 1); i++){
		readStructEEPROM(32 * i); // read I2C eeprom
		voltage = met.battery;
    humidityDHT22 = met.humidityDHT22 ;
    temperatureDHT22 = met.temperatureDHT22 ;
    p0 = met.externalPressure ;
		printWEB(false);// false add time from last web server record (recorded record)
		delay(5000);
	}
	writeEEPROM(nValuesAddr,0); //reset storage records nr on I2C eeprom
}
//WIFI
uint8_t connLAN()
{
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

		double gamma = log(humidityDHT22 / 100) + ((17.62 * temperatureDHT22) / (243.5 + temperatureDHT22));
	  dp = 243.5 * gamma / (17.62 - gamma);
	  double Humidex = temperatureDHT22 + (5 * ((6.112 * pow( 10, 7.5 * temperatureDHT22/(237.7 + temperatureDHT22))*humidityDHT22/100) - 10))/9;
		Serial.println("connected");
    // Make a HTTP request:
    String s =String("GET /meteofeletto/swpi_logger.php?temp_out=" + String(temperatureDHT22) +
    +"&&pwd=admin" +
    +"&&hum_out=" + String(humidityDHT22) +
    +"&&rel_pressure=" + String(p0) +
    +"&&dwew=" + String(dp) +
    +"&&humidex=" + String(Humidex) +
    +"&&voltage=" + String(voltage) +
		+"&&time=" + String(timeAvailable) +
    + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
    Serial.println(s);
    //c.println(s);

}

//I2C EEPROM
byte writeStructEEPROM(unsigned int addr)
{
	byte err;
	Wire.beginTransmission(SLAVE_ADDRESS);
	Wire.write ((byte) (addr >> 8));    // high order byte
	Wire.write ((byte) (addr & 0xFF));  // low-order byte
	I2C_writeAnything (met);
	err = Wire.endTransmission ();
	delay(6);  // needs 5ms for page write
	SERIAL_OUT.println("\nSCRITTI VALORI A  " + String(addr));
	SERIAL_OUT.println("batteria " + String(met.battery));
	SERIAL_OUT.println("hum " + String(met.humidityDHT22));
	SERIAL_OUT.println("temp " + String(met.temperatureDHT22));
	SERIAL_OUT.println("press " + String(met.externalPressure));
	//SERIAL_OUT.println("misura " + String(met(met)));
	SERIAL_OUT.println("misura ritornata " + String(sizeof(met)));
	return err;  // cannot write to device
}
byte readStructEEPROM(unsigned int addr)
{
  byte err;
	met.battery = 0;
	met.humidityDHT22 = 0;
	met.temperatureDHT22 = 0;
	met.externalPressure = 0;
	Wire.beginTransmission (SLAVE_ADDRESS);
  Wire.write ((byte) (addr >> 8));    // high order byte
  Wire.write ((byte) (addr & 0xFF));  // low-order byte
  err = Wire.endTransmission ();
	// initiate blocking read into internal buffer
  Wire.requestFrom (SLAVE_ADDRESS, sizeof(met));

  I2C_readAnything (met);
	SERIAL_OUT.println("\nRECUPERATI VALORI A  " + String(addr));
	SERIAL_OUT.println("batteria " + String(met.battery));
	SERIAL_OUT.println("hum " + String(met.humidityDHT22));
	SERIAL_OUT.println("temp " + String(met.temperatureDHT22));
	SERIAL_OUT.println("press " + String(met.externalPressure));
	//SERIAL_OUT.println("misura " + String(met(met)));
	SERIAL_OUT.println("misura ritornata " + String(sizeof(met)));

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
