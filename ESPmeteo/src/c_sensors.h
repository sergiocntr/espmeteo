/**
 * @file c_sensors.h
 * @brief Sensor data handling for BME280 and battery monitoring via ATtiny.
 */

#pragma once

#include <BME280I2C.h>
#include <Wire.h>

class SensorDataHandler {
private:
  BME280I2C::Settings settings;
  BME280I2C bme;
  meteoData currmet;

  /**
   * @brief Calculates a simple checksum for the data packet.
   */
  void calculateChecksum() {
    uint8_t checksum = 0;
    checksum += static_cast<uint16_t>(currmet.humidityBMP);
    checksum += static_cast<uint16_t>(abs(currmet.temperatureBMP));
    checksum += static_cast<uint16_t>(currmet.externalPressure);
    checksum += (currmet.battery >> 8) & 0xFF; // High byte
    checksum += currmet.battery & 0xFF;        // Low byte
    currmet.checksum = checksum & 0xFF;
  }

  /**
   * @brief Request battery voltage from ATtiny via I2C.
   */
  ErrorState requestVoltageFromAttiny() {
    uint8_t bytesRead = 0;
    uint16_t voltage;
    uint8_t dati[2] = {0};

    if (Wire.requestFrom(SLAVE_ATTINY_ADDRESS, 2) != 2) {
      ERROR_PRINT(WIREError_className, WIREError, WIREError::BYTE_NUM_ERROR);
      return ATTINY_BYTE_NUM_ERROR;
    }

    while (Wire.available() && bytesRead < 2) {
      dati[bytesRead++] = Wire.read();
    }

    if (bytesRead != 2) {
      ERROR_PRINT(WIREError_className, WIREError, WIREError::DATA_ERROR);
      return ATTINY_DATA_ERROR;
    }

    voltage = (dati[1] << 8) | dati[0];
    currmet.battery = voltage;

    sprintf(helpMet::str, "Voltage: %u mV", voltage);
    DEBUG_PRINT(helpMet::str);

    return NO_ERROR;
  }

  /**
   * @brief Reads data from the BME280 sensor.
   */
  ErrorState readFromBME280() {
    float temp = NAN, hum = NAN, pres = NAN;

    bme.read(pres, temp, hum);
    delay(200);
    bme.read(pres, temp, hum);
    delay(200);

    if (isnan(temp) || isnan(hum) || isnan(pres)) {
      return BME280_DATA_ERROR;
    }

    const float TEMP_OFFSET = 50.00; // Offset for negative temperatures
    temp += TEMP_OFFSET;

    currmet.humidityBMP = static_cast<int16_t>(hum * (1 << 7));       // x128
    currmet.temperatureBMP = static_cast<int16_t>(temp * (1 << 7));   // x128
    currmet.externalPressure = static_cast<int16_t>(pres * (1 << 5)); // x32

    return NO_ERROR;
  }

public:
  SensorDataHandler()
      : settings(BME280::OSR_X1, BME280::OSR_X1, BME280::OSR_X1,
                 BME280::Mode_Forced, BME280::StandbyTime_1000ms,
                 BME280::Filter_Off, BME280::SpiEnable_False),
        bme(settings) {}

  void initialize(ErrorState &error) {
    uint32_t startTime = millis();
    while (!bme.begin()) {
      if (millis() - startTime > 3000) {
        error = BME280_CONNECTION_FAILED;
        return;
      }
      delay(400);
    }

    currmet.deviceID = deviceIDs::marinerUno;
    currmet.battery = 0;
    currmet.checksum = 0;
    currmet.counter = 0;
    currmet.externalPressure = 0;
    currmet.humidityBMP = 0;
    currmet.temperatureBMP = 0;

    error = NO_ERROR;
  }

  /**
   * @brief Populates the provided structure with fresh sensor data.
   */
  ErrorState getRecord(meteoData *mdata) {
    ErrorState errCode = NO_ERROR;
    int retryCount = 3;

    while (retryCount > 0) {
      errCode = readFromBME280();
      if (errCode == NO_ERROR)
        break;
      retryCount--;
      delay(1000);
    }

    if (errCode > 0) {
      DEBUG_PRINT("BME280 Read Error");
      return errCode;
    }

    errCode = requestVoltageFromAttiny();
    if (errCode > 0) {
      DEBUG_PRINT("ATtiny Voltage Request Error");
      return errCode;
    }

    calculateChecksum();
    DEBUG_PRINT("Sensor suite OK");

    *mdata = currmet;
    return NO_ERROR;
  }

  /**
   * @brief Generates a dummy record for testing.
   */
  ErrorState getDummyRecord(meteoData *mdata) {
    float hum = random(40, 60) + random(0, 99) / 100.0;
    float temp = random(20, 30) + random(0, 99) / 100.0;
    int pres = random(970, 1041);

    currmet.deviceID = deviceIDs::marinerUno;
    currmet.humidityBMP = static_cast<int16_t>(hum * (1 << 7));
    currmet.temperatureBMP = static_cast<int16_t>(temp * (1 << 7));
    currmet.externalPressure = static_cast<int16_t>(pres * (1 << 7));
    currmet.battery = random(3950, 4451);
    currmet.checksum = 0;
    currmet.counter = 0;

    calculateChecksum();
    DEBUG_PRINT("Dummy Data generated");

    *mdata = currmet;
    return NO_ERROR;
  }
};
