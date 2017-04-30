#include <Wire.h>
#include <ESP8266WiFi.h>
//#include <LiquidCrystal_I2C.h>
static int default_sda_pin = 0;
static int default_scl_pin = 2;
//Wifi Settings
WiFiClient c;
const char* ssid     = "TIM-18232399";
const char* password = "ObXtYwlWaqnXIJjqs2NbF6qP";
IPAddress ip(192, 168, 1, 211); //Node static IP
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
//web host Settings
#define host "www.developteamgold.altervista.org"
const int httpPort = 80;
uint16_t voltage = 0;
uint8_t dati[3];
void setup()
{
Serial.begin(9600);
  connLAN();  //mi collego a alla rete per mettere in sforzo la batteria
  //Serial.begin(115200);
  delay(500); // do tempo a Attiny di leggere la tensione
  Wire.begin(default_sda_pin, default_scl_pin);
    for (int i=0; i <= 2; i++){
      Wire.requestFrom(2, 2);    // request 2 bytes from slave device #2---- Wire.requestFrom (SLAVE_ADDRESS, responseSize);
    //volatile int ref =0;
      dati[i] = Wire.read();    // receive a byte as character
      //ref++;
      //Serial.print(c);         // print the character
    }
  /*  int i=0;
    Wire.requestFrom(2, 2);    // request 2 bytes from slave device #2---- Wire.requestFrom (SLAVE_ADDRESS, responseSize);
    while(Wire.available())    // slave may send less than requested
  {
    dati[i] = Wire.read();    // receive a byte as character
    i++;         // print the character
  }*/
    voltage = (dati[1]<<8) | dati[0];
    //voltage = dati[0];

    printWEB();
    //delay(500);
  Wire.begin(default_sda_pin, default_scl_pin);
  Wire.beginTransmission (2);
  Wire.write (20);
  Wire.endTransmission(true);
  delay(5);

}
void loop(){

//Wire.write (20);
//*/
}
void connLAN()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.config(ip, gateway, subnet); // Set static IP (2,7s) or 8.6s with DHCP  + 2s on battery
  while (WiFi.status() != WL_CONNECTED) {
    //digitalWrite(ENABLE_PIN, LOW);
    Serial.println("connecting lan....");
    delay(500);
    //digitalWrite(ENABLE_PIN, HIGH);
  }
  Serial.println("Lan OK!");
}
void printWEB()
{
  if (c.connect(host, httpPort))
  {
    Serial.println("connected");
    // Make a HTTP request:
    String s =String("GET /meteofeletto/swpi_logger.1.php?voltage=" + String(voltage) +
    +"&&pwd=admin" +
    +"&&lowbyte=" + String(dati[0]) +
    +"&&highbyte=" + String(dati[1]) +
    //+"&&humidex=" + String(dati[2]) +
    + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
    Serial.println(s);
    c.println(s);
  }

}
