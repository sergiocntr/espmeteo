//#define ALTITUDE 110.0 // Altitude
void sensor_init(){
  dht.begin();						//DTH22 initialization
	bmp.begin();
}
void dh()
{
  //DHT22 stuff
  // Wait a few seconds between measurements.
  //
  delay(2000);
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  humidityDHT22 = dht.readHumidity();

  while (isnan(humidityDHT22)) {
    Serial.println("Error reading humidity!");
    delay(2000);
    humidityDHT22 = dht.readHumidity();
  }
  temperatureDHT22 = dht.readTemperature();
  while (isnan(temperatureDHT22)) {
    Serial.println("Error reading temperature!");
    delay(2000);
    temperatureDHT22 = dht.readTemperature();
  }

}
void bm()
{
  int ALTEZZA_MISURA_MT = 125;
  bmp.readTemperature();
  //p0=bme.readPressure();
  p0 = bmp.readPressure() / 100.0;
  p0 = ( p0 / pow( 1 - ( ALTEZZA_MISURA_MT / 44330.0 ) , 5.255 ) );
}

/*void printSerial()
{
  Serial.println();
  Serial.print("provided altitude: ");
  Serial.print(ALTITUDE,0);
  Serial.print(" meters, ");
  Serial.print(ALTITUDE*3.28084,0);
  Serial.println(" feet");
  Serial.print("temperature: ");
  Serial.print(T,2);
  Serial.print(" deg C, ");
  Serial.print((9.0/5.0)*T+32.0,2);
  Serial.println(" deg F");
  Serial.print("absolute pressure: ");
  Serial.print(P,2);
  Serial.print(" mb, ");
  Serial.print(P*0.0295333727,2);
  Serial.println(" inHg");
  p0 = pressure.sealevel(P,ALTITUDE); // we're at 1655 meters (Boulder, CO)
  Serial.print("relative (sea-level) pressure: ");
  Serial.print(p0,2);
  Serial.print(" mb, ");
  Serial.print(p0*0.0295333727,2);
  Serial.println(" inHg");
  a = pressure.altitude(P,p0);
  Serial.print("computed altitude: ");
  Serial.print(a,0);
  Serial.print(" meters, ");
  Serial.print(a*3.28084,0);
  Serial.println(" feet");
}*/
