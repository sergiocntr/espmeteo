/*
questo e' il codice che gira sull ESP meteo
aggiorna il voltaggio
spegne ESP
*/
#include <Wire.h>
//ESP-01 SDA - SCL pin
static int default_sda_pin = 0;
static int default_scl_pin = 2;
//I2C eeprom stuff
const int SLAVE_ADDRESS = 0X50; //classic I2C EEPROM address
const uint16_t nValuesAddr = 0x0FFF; //address on I2C EEPROM ,we store there how many reecords we have
//structure used to store and recall data
#include <I2C_Anything.h>
struct meteoData{
	float humidityDHT22;
	float temperatureDHT22;
	double externalPressure;
  int battery;
};
typedef struct meteoData MeteoData;
MeteoData met, retmet;
//WIFI stuff
#include <ESP8266WiFi.h>
#include <ESP8266Ping.h> //used to know if we are online
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
uint8_t dati[2];        // attiny low and high voltage byte
//BMP stuff
#include <SFE_BMP180.h>
SFE_BMP180 pressure;
char status;
double T,P,p0,a;
//DHT22 stuff
#include "DHT.h"
#define DHTPIN 3  //GPIO3 (rx) what digital pin we're connected to- was 1 : tx needed for debug info
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);
float humidityDHT22,temperatureDHT22,Humidex,dp;

//keep in mind that the ESP is rebooted every 15 min by the Attiny sleep timer
void setup()
{
	//begin section
	Serial.begin(9600);
	//Start I2C comm
	Wire.begin(default_sda_pin, default_scl_pin);
  delay(500); 									// do tempo a Attiny di leggere la tensione
	//check Wifi local lan connection  1 -> connected to local WIFI ; 0-> offline
  uint8_t check = connLAN();
	//check if we have  meteo records stored in I2C
	uint8_t value = readEEPROM(nValuesAddr);
	//if there are meteo record and now we are on line send the stored data to the web server
	if(check == 1 && value > 0) sendData(value); // if WIFI available and records stored, send them to server
	//data section
	requestSensorsValues();
  if(check == 1){	//  if local WIFI connection OK send data without save to I2C

		printWEB(true); // send data to server (true = get time from web server (live record))
	}
  else{		//local WIFI connection KO
		uint16_t availAddress = 32 * value;	//MeteoData is 24 bytes long so..
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
  //Serial.println("spegniti");
  //Wire.begin(default_sda_pin, default_scl_pin);
  Wire.beginTransmission (2);
  Wire.write (20);
  Wire.endTransmission(true);
  delay(250);
}
//data
void requestSensorsValues(){
	while (voltage < 3500 | voltage > 5000){			//sanity check about voltage value
    //Wire.begin(default_sda_pin, default_scl_pin);		//better way welcome!
    for (int i=0; i <= 2; i++){
      Wire.requestFrom(2, 2);    // request 2 bytes from slave device #2---- Wire.requestFrom (SLAVE_ADDRESS, responseSize);
      dati[i] = Wire.read();    // receive a byte as character
    }
    voltage = (dati[1]<<8) | dati[0];
  }
	Serial.printl("Dati 1 : " + dati[1] + " dati 0 : " + dati[0]);
	Serial.println("Battery : " + String(voltage));
  //delay(50);							//I'll try to avoid those delays....
  dht.begin();						//DTH22 initialization
  //delay(50);
  pressure.begin();			//BMP080 initialization
  bm();									// read BMP080 values
  Serial.println("Pressure : " + String(p0));
  //delay(2000);
  dh();									//read DHT22 values
  Serial.println("Temp : " + String(temperatureDHT22));
	Serial.println("Hum : " + String(humidityDHT22));
  //delay(50);
  //Serial.println("connesso lan" );
}
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
//WIFI
uint8_t connLAN()
{
  uint8_t check=0;
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.config(ip, gateway, subnet); // Set static IP (2,7s) or 8.6s with DHCP  + 2s on battery
  for (int i=0; i <= 5; i++){
    if(WiFi.status() == WL_CONNECTED)
    {
			check = 1;
			break;
		}
    delay(700);
  }

	if (check == 0) WiFi.mode(WIFI_OFF); //energy saving mode if local WIFI isn't connected
	else
	{
		for (int i = 0; i < 5; i++) {
			if(Ping.ping("www.gogle.com"))
			{
				check = 1;
				break;
		  } else
			{
		    check = 0;
		  }
			delay(2000);
		}
	}

		return check;
}
void printWEB(bool timeAvailable) //timeAvailable -> live mesaures
{
  if (c.connect(host, httpPort))
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
    c.println(s);
  }
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
