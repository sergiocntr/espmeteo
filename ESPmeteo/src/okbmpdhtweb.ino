//#define ALTITUDE 110.0 // Altitude
void sensor_init(){
  dht.begin();						//DTH22 initialization
	bmp.begin();
}
void dh()
{
  //DHT22 stuff
  // Wait a few seconds between measurements.
  humidityDHT22 =0;
  temperatureDHT22=0;
  for (int mis = 0; mis < 5; mis++) {
    delay(2000);
    float hum = dht.readHumidity();
    float temp =dht.readTemperature();
    while (isnan(hum) | isnan(temp)) {
      Serial.println("Error reading DHT22!");
      delay(2000);
      hum = dht.readHumidity();
      temp =dht.readTemperature();
    }
    humidityDHT22 += hum;
    temperatureDHT22 += temp;
    #ifdef DEBUG
    Serial.print(F("hum = "));
    Serial.println(hum);
    Serial.print(F("temp = "));
    Serial.println(temp);
    Serial.print(F("somma hum = "));
    Serial.println(humidityDHT22);
    Serial.print(F("somma temp = "));
    Serial.println(temperatureDHT22);
    #endif
  }
  temperatureDHT22 /= 5;
  humidityDHT22 /= 5;
}
void bm()
{
  int ALTEZZA_MISURA_MT = 125;

  p0 = 0;
  //bmp.readTemperature();
  //p0=bme.readPressure();
  //double p = bmp.readPressure();

  for (int mis = 0; mis < 5; mis++) {
    delay(1000);
    bmp.readTemperature();
    //p0=bme.readPressure();
    double p = bmp.readPressure() / 100.0;
    p = ( p / pow( 1 - ( ALTEZZA_MISURA_MT / 44330.0 ) , 5.255 ) );
    Serial.print(F("Pressure = "));
    Serial.println(p);
    p0  += p;
    Serial.print(F("Somma = "));
    Serial.println(p0);

    /*Serial.print(F("Temperature = "));
    Serial.print(bmp.readTemperature());
    Serial.println(" *C");

    Serial.print(F("Pressure = "));
    Serial.print(bmp.readPressure());
    Serial.println(" Pa");

    Serial.print(F("Approx altitude = "));
    Serial.print(bmp.readAltitude(1013.25)); // this should be adjusted to your local forcase
    Serial.println(" m");

    Serial.println();*/
  }
  p0 /= 5;
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
