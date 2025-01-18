#include <BME280I2C.h>
#include <Wire.h>
class SensorDataHandler
{
private:
  BME280I2C::Settings settings; // Variabile membro per i settings del sensore
  BME280I2C bme;

  meteoData currmet;

  void calculateChecksum()
  {
    uint8_t checksum = 0;

    checksum += static_cast<uint16_t>(currmet.humidityBMP * 100);
    checksum += static_cast<uint16_t>(currmet.temperatureBMP * 100);
    checksum += static_cast<uint16_t>(currmet.externalPressure * 10);
    checksum += (currmet.battery >> 8) & 0xFF;
    checksum += currmet.battery & 0xFF;

    currmet.checksum = checksum;
    
  }

  

  // Funzione per leggere i dati dal sensore BME280
  ErrorState readFromBME280()
  {
    float temp = NAN, hum = NAN, pres = NAN;

    bme.read(pres, temp, hum);
    delay(200);
    bme.read(pres, temp, hum);
    delay(200);
    if (isnan(temp) || isnan(hum) || isnan(pres))
    {
      //ERROR_PRINT(SENSORError_className, SENSORError, SENSORError::BME280_DATA_ERROR);
      return BME280_DATA_ERROR; // Errore nei dati
    }

    currmet.humidityBMP = hum;
    currmet.temperatureBMP = temp;
    currmet.externalPressure = pres;

// #if defined DEBUGMIO
//     Serial.print("Temperatura: ");
//     Serial.println(currmet.temperatureBMP);
//     Serial.print("Umidità: ");
//     Serial.println(currmet.humidityBMP);
//     Serial.print("Pressione: ");
//     Serial.println(currmet.externalPressure);
// #endif
    
    return NO_ERROR; // Successo
  }

public:
  // Costruttore che inizializza il sensore BME280 con i settings
  SensorDataHandler()
      : settings(BME280::OSR_X1,             // Sovracampionamento temperatura
                 BME280::OSR_X1,             // Sovracampionamento pressione
                 BME280::OSR_X1,             // Sovracampionamento umidità
                 BME280::Mode_Forced,        // Modalità: Forzata (misura su richiesta)
                 BME280::StandbyTime_1000ms, // Tempo di standby
                 BME280::Filter_Off,         // Filtro disabilitato
                 BME280::SpiEnable_False),   // Comunicazione I2C
        bme(settings)                        // Inizializzazione dell'oggetto BME280I2C con i settings
  {
    // Altre inizializzazioni
    // eBufferAddress = circBuf::readLastPageAddressAndRecordCount(eBuffer);
    // eBuffer.recordCount = 0;
  }

  void  initialize(ErrorState &error){
    uint32_t startTime = millis();
    while (!bme.begin())
    {
      if (millis() - startTime > 3000)
      {
        //ERROR_PRINT(SENSORError_className, SENSORError, SENSORError::BME280_CONNECTION_FAILED);
        error = BME280_CONNECTION_FAILED; // Errore di inizializzazione
        return;
      }
      delay(400);
    }
    currmet.battery = 0;
    currmet.checksum = 0;
    currmet.counter = 0;
    currmet.externalPressure = 0.0;
    currmet.humidityBMP = 0.00;
    currmet.temperatureBMP = 0.00;

    error = NO_ERROR;


  }


  // Getter per il record
  ErrorState getDummyRecord(meteoData *mdata)
  {
    ErrorState errCode = NO_ERROR;
    
    // Simulazione dati sensore
		currmet.temperatureBMP = random(20, 30) + random(0, 99) / 100.0;
		currmet.humidityBMP = random(40, 60) + random(0, 99) / 100.0;
		currmet.externalPressure = random(970, 1041); // Pressione atmosferica simulata
		currmet.battery = random(3950, 4451);		  // Livello batteria simulato
		currmet.checksum = 0;					  // checksum
		currmet.counter = 0;						  // counter

    calculateChecksum();

    

    DEBUG_PRINT("BME dummy tutto ok!!");

    *mdata = currmet; // Copia dei dati nella struttura fornita
    return errCode;         // Tutto OK
  }
};
