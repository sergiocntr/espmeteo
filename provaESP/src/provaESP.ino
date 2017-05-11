#include "DHT.h"
#include <SFE_BMP180.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
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
uint16_t voltage = 0;
uint8_t dati[2];
//BMP stuff
SFE_BMP180 pressure;
char status;
double T,P,p0,a;
//DHT22 stuff
#define DHTPIN 1  //GPIO1 (Tx) what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);
float humidityDHT22,temperatureDHT22,Humidex,dp;

void setup()
{
Serial.begin(9600);
  delay(500); // do tempo a Attiny di leggere la tensione
  connLAN();
  while (voltage < 3500 | voltage > 5000){
    Wire.begin(default_sda_pin, default_scl_pin);
    for (int i=0; i <= 2; i++){
      Wire.requestFrom(2, 2);    // request 2 bytes from slave device #2---- Wire.requestFrom (SLAVE_ADDRESS, responseSize);
      dati[i] = Wire.read();    // receive a byte as character
    }
    voltage = (dati[1]<<8) | dati[0];
  }
  delay(50);
  dht.begin();
  delay(50);
  pressure.begin();
  bm();
  Serial.println("recuperata pressione : " + String(p0));
  delay(2000);
  dh();
  Serial.println("recuperata temperatura/um  : " + String(temperatureDHT22));
  delay(50);
  Serial.println("connesso lan" );
  printWEB();
  Serial.println("trasmesso web" );
}
void loop(){
  Serial.println("spegniti");
  Wire.begin(default_sda_pin, default_scl_pin);
  Wire.beginTransmission (2);
  Wire.write (20);
  Wire.endTransmission(true);
  delay(250);

}
void connLAN()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.config(ip, gateway, subnet); // Set static IP (2,7s) or 8.6s with DHCP  + 2s on battery
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("connecting lan....");
    delay(500);
  }
  //Serial.println("Lan OK!");
}
void printWEB()
{
  if (c.connect(host, httpPort))
  {
    Serial.println("connected");
    // Make a HTTP request:
    String s =String("GET /meteofeletto/swpi_logger.php?temp_out=" + String(temperatureDHT22) +
    +"&&pwd=admin" +
    +"&&hum_out=" + String(humidityDHT22) +
    +"&&rel_pressure=" + String(p0) +
    +"&&dwew=" + String(dp) +
    +"&&humidex=" + String(Humidex) +
    +"&&voltage=" + String(voltage) +
    + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
    Serial.println(s);
    c.println(s);
  }
}
