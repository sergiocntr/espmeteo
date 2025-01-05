#include <BME280I2C.h>
#include <Wire.h>
class SensorDataHandler
{
private:
  BME280I2C::Settings settings; // Variabile membro per i settings del sensore
  BME280I2C bme;

  meteoData currmet;
  uint8_t buffer[sizeof(meteoData)];
  uint16_t eBufferAddress;
  // Funzione per calcolare il checksum
  uint8_t calculateChecksum()
  {
    uint8_t checksum = 0;

    checksum += static_cast<uint16_t>(currmet.humidityBMP * 100);
    checksum += static_cast<uint16_t>(currmet.temperatureBMP * 100);
    checksum += static_cast<uint16_t>(currmet.externalPressure * 10);
    checksum += (currmet.battery >> 8) & 0xFF;
    checksum += currmet.battery & 0xFF;

    currmet.checksum = checksum;
    return 0;
  }

  // Funzione per leggere la tensione dall'Attiny
  uint8_t requestVoltageFromAttiny()
  {

    uint8_t bytesRead = 0;
    uint16_t voltage;
    int length = sizeof(voltage);
    uint8_t dati[length] = {0};

    if (Wire.requestFrom(SLAVE_ATTINY_ADDRESS, length) != length)
    {
      ERROR_PRINT(WIREError_className, WIREError, WIREError::BYTE_NUM_ERROR);
      return 3;
    }

    while (Wire.available() && bytesRead < length)
    {
      dati[bytesRead++] = Wire.read();
    }

    if (bytesRead != length)
    {
      ERROR_PRINT(WIREError_className, WIREError, WIREError::DATA_ERROR);
      return 4;
    }

    voltage = (dati[1] << 8) | dati[0];

    currmet.battery = voltage;

    sprintf(helpMet::str, "Tensione %u millivolt.", voltage);
    DEBUG_PRINT(helpMet::str);

    return 0;
  }

  // Funzione per leggere i dati dal sensore BME280
  uint8_t readFromBME280()
  {
    float temp = NAN, hum = NAN, pres = NAN;

    uint32_t startTime = millis();
    while (!bme.begin())
    {
      if (millis() - startTime > 3000)
      {
        ERROR_PRINT(SENSORError_className, SENSORError, SENSORError::BME280_CONNECTION_FAILED);
        return 1; // Errore di inizializzazione
      }
      delay(400);
    }

    bme.read(pres, temp, hum);
    delay(200);
    bme.read(pres, temp, hum);
    delay(200);
    if (isnan(temp) || isnan(hum) || isnan(pres))
    {
      ERROR_PRINT(SENSORError_className, SENSORError, SENSORError::BME280_DATA_ERROR);
      return 2; // Errore nei dati
    }

    currmet.humidityBMP = hum;
    currmet.temperatureBMP = temp;
    currmet.externalPressure = pres;

#if defined DEBUGMIO
    Serial.print("Temperatura: ");
    Serial.println(currmet.temperatureBMP);
    Serial.print("Umidità: ");
    Serial.println(currmet.humidityBMP);
    Serial.print("Pressione: ");
    Serial.println(currmet.externalPressure);
#endif
    
    return 0; // Successo
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

  // Getter per il record
  uint8_t getRecord(meteoData *mdata)
  {
    uint8_t errCode = readFromBME280();

    if (errCode > 0)
    {
      DEBUG_PRINT("Errore nel leggere il BME280");
      return errCode;
    }

    errCode = requestVoltageFromAttiny();

    if (errCode > 0)
    {
      DEBUG_PRINT("Errore nel richiedere il voltaggio dall'Attiny");
      return errCode;
    }

    errCode = calculateChecksum();

    if (errCode > 0)
    {
      DEBUG_PRINT("Errore nel calcolo del checksum");
      return errCode;
    }

    DEBUG_PRINT("BME tutto ok!!");

    *mdata = currmet; // Copia dei dati nella struttura fornita
    return 0;         // Tutto OK
  }
  // Getter per il record
  uint8_t getDummyRecord(meteoData *mdata)
  {
    // Simulazione dati sensore
		currmet.temperatureBMP = random(20, 30) + random(0, 99) / 100.0;
		currmet.humidityBMP = random(40, 60) + random(0, 99) / 100.0;
		currmet.externalPressure = random(970, 1041); // Pressione atmosferica simulata
		currmet.battery = random(3950, 4451);		  // Livello batteria simulato
		currmet.checksum = 0;					  // checksum
		currmet.counter = 0;						  // counter

    uint8_t errCode = calculateChecksum();

    if (errCode > 0)
    {
      DEBUG_PRINT("Errore nel calcolo del checksum dummy");
      return errCode;
    }

    DEBUG_PRINT("BME dummy tutto ok!!");

    *mdata = currmet; // Copia dei dati nella struttura fornita
    return 0;         // Tutto OK
  }
};
