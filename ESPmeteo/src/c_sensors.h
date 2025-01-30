#include <BME280I2C.h>
#include <Wire.h>
class SensorDataHandler
{
private:
  BME280I2C::Settings settings; // Variabile membro per i settings del sensore
  BME280I2C bme;

  meteoData currmet;

  void calculateChecksum() {
    uint8_t checksum = 0;

    // Aggiungi i valori direttamente come interi
    checksum += static_cast<uint16_t>(currmet.humidityBMP);        // Umidità come int16_t (valore positivo)
    checksum += static_cast<uint16_t>(abs(currmet.temperatureBMP)); // Temperatura come int16_t (assoluto)
    checksum += static_cast<uint16_t>(currmet.externalPressure);   // Pressione come int16_t (valore positivo)

    // Somma i due byte della batteria separatamente
    checksum += (currmet.battery >> 8) & 0xFF;                     // Byte alto della batteria
    checksum += currmet.battery & 0xFF;                             // Byte basso della batteria

    currmet.checksum = checksum & 0xFF; // Mantieni solo gli ultimi 8 bit
}

  // Funzione per leggere la tensione dall'Attiny
  ErrorState requestVoltageFromAttiny()
  {

    uint8_t bytesRead = 0;
    uint16_t voltage;
    int length = sizeof(voltage);
    uint8_t dati[length] = {0};

    if (Wire.requestFrom(SLAVE_ATTINY_ADDRESS, length) != length)
    {
      ERROR_PRINT(WIREError_className, WIREError, WIREError::BYTE_NUM_ERROR);
      return ATTINY_BYTE_NUM_ERROR;
    }

    while (Wire.available() && bytesRead < length)
    {
      dati[bytesRead++] = Wire.read();
    }

    if (bytesRead != length)
    {
      ERROR_PRINT(WIREError_className, WIREError, WIREError::DATA_ERROR);
      return ATTINY_DATA_ERROR;
    }

    voltage = (dati[1] << 8) | dati[0];

    currmet.battery = voltage;

    sprintf(helpMet::str, "Tensione %u millivolt.", voltage);
    DEBUG_PRINT(helpMet::str);

    return NO_ERROR;
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
      // ERROR_PRINT(SENSORError_className, SENSORError, SENSORError::BME280_DATA_ERROR);
      return BME280_DATA_ERROR; // Errore nei dati
    }

    currmet.humidityBMP = static_cast<int16_t>(hum * (1 << 7));       // Moltiplicazione con shift (x128)
    currmet.temperatureBMP = static_cast<int16_t>(temp * (1 << 7));   // Moltiplicazione con shift (x128)
    currmet.externalPressure = static_cast<int16_t>(pres * (1 << 5)); // Moltiplicazione con shift (x32)

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

  void initialize(ErrorState &error)
  {
    uint32_t startTime = millis();
    while (!bme.begin())
    {
      if (millis() - startTime > 3000)
      {
        // ERROR_PRINT(SENSORError_className, SENSORError, SENSORError::BME280_CONNECTION_FAILED);
        error = BME280_CONNECTION_FAILED; // Errore di inizializzazione
        return;
      }
      delay(400);
    }
    currmet.battery = 0;
    currmet.checksum = 0;
    currmet.counter = 0;
    currmet.externalPressure = 0;
    currmet.humidityBMP = 0;
    currmet.temperatureBMP = 0;

    error = NO_ERROR;
  }

  // Getter per il record
  ErrorState getRecord(meteoData *mdata)
  {
    ErrorState errCode = NO_ERROR;
    int retryCount = 3; // Numero massimo di tentativi per la lettura del sensore
    while (retryCount > 0)
    {
      errCode = readFromBME280();
      if (errCode == NO_ERROR)
      {
        break; // Lettura riuscita, esci dal ciclo
      }
      retryCount--;
      delay(1000); // Attendere prima di riprovare
    }

    if (errCode > 0)
    {
      DEBUG_PRINT("Errore nel leggere il BME280");
      return errCode; // Restituisce l'errore di lettura della sonda
    }

    errCode = requestVoltageFromAttiny();

    if (errCode > 0)
    {
      DEBUG_PRINT("Errore nel richiedere il voltaggio dall'Attiny");
      return errCode;
    }

    calculateChecksum();

    // if (errCode > 0)
    // {
    //   DEBUG_PRINT("Errore nel calcolo del checksum");
    //   return errCode;
    // }

    DEBUG_PRINT("BME tutto ok!!");

    *mdata = currmet; // Copia dei dati nella struttura fornita
    return NO_ERROR;  // Tutto OK
  }
  // Getter per il record
  ErrorState getDummyRecord(meteoData *mdata)
  {
    ErrorState errCode = NO_ERROR;

    // Simulazione dati sensore
    // Genera i valori casuali e applica la conversione
    float hum = random(40, 60) + random(0, 99) / 100.0;  // Umidità casuale
    float temp = random(20, 30) + random(0, 99) / 100.0; // Temperatura casuale
    int pres = random(970, 1041);                        // Pressione atmosferica simulata

    // Moltiplicazione e conversione in interi
    currmet.humidityBMP = static_cast<int16_t>(hum * (1 << 7));       // Moltiplicazione con shift (x128)
    currmet.temperatureBMP = static_cast<int16_t>(temp * (1 << 7));   // Moltiplicazione con shift (x128)
    currmet.externalPressure = static_cast<int16_t>(pres * (1 << 7)); // Moltiplicazione con shift (x128)
    currmet.battery = random(3950, 4451);                             // Livello batteria simulato
    currmet.checksum = 0;                                             // checksum
    currmet.counter = 0;                                              // counter

    calculateChecksum();

    DEBUG_PRINT("BME dummy tutto ok!!");

    *mdata = currmet; // Copia dei dati nella struttura fornita
    return errCode;   // Tutto OK
  }
};
