#include "main.h"
void checkI2C()
{
  //CONTROLLA IL BUS I2C PER DISPOSITIVI
  char str[80];
  byte error, address;
  int nDevices;
  Serial.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
 
    if (error == 0)
    {
      // Serial.print("I2C device found at address 0x");
      // if (address<16)
      //   Serial.print("0");
      // Serial.print(address,HEX);
      // Serial.println("  !");
      sprintf(str, "Trovato device i2C indirizzo  %u", address);
      Serial.println(str);
      publish(logTopic,str);
      smartDelay(100);
      nDevices++;
    }
    else if (error==4)
    {
      Serial.print("Unknown error at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done");
 
  //delay(5000);           // wait 5 seconds for next scan
}
void checkEEPROM(){
  //FA UNA VERIFICA DI LETTURA SCRITTURA DI CELLE DELLA MEMORIA I2C
	uint8_t testArray [5] = { 16, 2, 77, 40, 234 }; 
    uint16_t testPos [5] = { 2, 20, 89, 32, 255 };
	for (size_t i = 0; i < 5; i++)
	{
		writeEEPROM(testPos[i],testArray[i]);
	}
	delay(100);
	for (size_t i = 0; i < 5; i++)
	{
		uint8_t myval= readEEPROM(testPos[i]);
		if(myval==testArray[i]){
			Serial.println("pos " + String(i) + String(" ok!"));
		}else{
			Serial.println("pos " + String(i) + String(" FAIL!"));
		}
		delay(250);
	}
}

void vediamoDati(){
  //SCRIVE E LEGGE UNA PAGINA DI MEMORIA
  byte ris;
  met.humidityBMP = 6.8;
	met.temperatureBMP = 2.4;
	met.externalPressure = 1005.25;
  met.battery=1234;
  for (uint16_t i = 0; i < 6; i++)
  {
    met.humidityBMP+= 10;
	  met.temperatureBMP += 10;
	  met.externalPressure += 10;
    met.battery+=10;
    delay(20);
    ris=writeStructEEPROM(32*i);
    Serial.println("scrittura " + String(ris));
    delay(250);
  }
  for (uint16_t i = 0; i < 6; i++)
  {
    ris= readStructEEPROM(32*i);
    Serial.println("lettura " + String(ris));
    Serial.println(met.humidityBMP);
    Serial.println(met.temperatureBMP);
    Serial.println(met.externalPressure);
    Serial.println(met.battery);
    delay(250);
  }
}

void readCells(){
  /*LEGGE TRENTASEI CELLE DI MEMORIA PER TROVARE IL VALORE AA
  SE NON TROVA AA LO SCRIVE NELLA POSIZIONE 1 E SCRIVE 0 NELLA CELLA DOPO
  SE LO TROVA SCRIVE IL VALORE DA MEMORIZZARE NELLA CELLA DOPO (AGGIUNGE UNO AL VALORE CHE GIA ESISTE)

  QUANDO RIPRENDE LA CONNESSIONE E MANDA TUTTI I VALORI SCRIVE 0 NELLA CELLA DOVE C'È' AA E SCRIVE AA NELLA CELLA DOVE C'E' IL VALORE DA CANCELLARE
   */
  const uint16_t nValuesAddr = 0x0FE0; //indirizzo memoria di partenza
  const uint8_t defaultValue = 0xAA; //valore di riferimento ,la cella attuale di memoria è quella dopo
  char str[80];
  for (uint16_t i = nValuesAddr; i < 0x0FFE; i++)
  {
    uint8_t myval= readEEPROM(i);
    if(myval == defaultValue) 
    {
      sprintf(str, "Trovato AA nella cella  %u", i);
      Serial.println(str);
      publish(logTopic,str);
      smartDelay(100);
    }

    writeEEPROM(i,0);

  }
  sprintf(str, "Cancellato celle");
  Serial.println(str);
  publish(logTopic,str);
  smartDelay(100);
  writeEEPROM(nValuesAddr,0xAA);
  sprintf(str, "Scritto AA in cella zero");
  Serial.println(str);
  publish(logTopic,str);
  smartDelay(100);
  for (uint16_t i = nValuesAddr; i < 0x0FFE; i++)
  {
    uint8_t myval= readEEPROM(i);
    if(myval == defaultValue) 
    {
      sprintf(str, "Trovato AA nella cella  %u", i);
      Serial.println(str);
      publish(logTopic,str);
      smartDelay(100);
    }
  }
}

void setup()
{ 
  wifi_set_sleep_type(LIGHT_SLEEP_T);
	WiFi.mode(WIFI_OFF);
	WiFi.forceSleepBegin();
	delay(1000);
	Serial.begin(9600);
	DEBUG_PRINT("Booting!");
	delay(3000);
  uint8_t a =collegaScheda();
  Serial.println(a);
	Wire.begin(default_sda_pin, default_scl_pin);
	checkI2C();
  delay(50);
  //checkEEPROM();
  delay(50);
  //vediamoDati();
  readCells();
}
void loop(){
  smartDelay(5000);
  //vediamoDati();
  //Serial.println();
}