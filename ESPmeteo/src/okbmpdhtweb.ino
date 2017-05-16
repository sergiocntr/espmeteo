#define ALTITUDE 110.0 // Altitude
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
    delay(2000);
    humidityDHT22 = dht.readHumidity();
  }
  temperatureDHT22 = dht.readTemperature();
  while (isnan(temperatureDHT22)) {
    delay(2000);
    humidityDHT22 = dht.readTemperature();
  }
}
void bm()
{
  status = pressure.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:
    delay(status);
    status = pressure.getTemperature(T);
    if (status != 0)
    {
      status = pressure.startPressure(3);
      if (status != 0)
      {
        // Wait for the measurement to complete:
        delay(status);
        status = pressure.getPressure(P,T);
        if (status != 0)
        {
            p0 = pressure.sealevel(P,ALTITUDE);
        }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");


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
