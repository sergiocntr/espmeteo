/**
 * **************************************************************************
 *
 *    ███████╗███████╗██████╗ ███╗   ███╗███████╗████████╗███████╗ ██████╗
 *    ██╔════╝██╔════╝██╔══██╗████╗ ████║██╔════╝╚══██╔══╝██╔════╝██╔═══██╗
 *    █████╗  ███████╗██████╔╝██╔████╔██║█████╗     ██║   █████╗  ██║   ██║
 *    ██╔══╝  ╚════██║██╔═══╝ ██║╚██╔╝██║██╔══╝     ██║   ██╔══╝  ██║   ██║
 *    ███████╗███████║██║     ██║ ╚═╝ ██║███████╗    ██║   ███████╗╚██████╔╝
 *    ╚══════╝╚══════╝╚═╝     ╚═╝     ╚═╝╚══════╝    ╚═╝   ╚══════╝ ╚═════╝
 *
 * **************************************************************************
 * @file        ESPmeteo.cpp
 * @project     ESPmeteo - Smart Weather Station
 * @brief       Collects atmospheric data (BME280) and sends it via MQTT.
 *              Supports offline logging to EEPROM when WiFi is unavailable.
 * @author      Sergio
 * @version     1.2
 * **************************************************************************
 */

#include "ESPmeteo.h"

extern "C" {
#include "user_interface.h"
}

/**
 * @brief Prints current free RAM to Serial console.
 */
void printFreeRAM(const char *label) {
  Serial.print(label);
  Serial.print(" - Free heap: ");
  Serial.println(ESP.getFreeHeap());
}

/**
 * @brief Sends a shutdown command to the ATtiny power manager.
 */
void shutDownNow() {
  Wire.beginTransmission(SLAVE_ATTINY_ADDRESS); // I2C address of ATtiny
  Wire.write(20);                               // Command 20: Power down
  Wire.endTransmission(true);
  delay(50);
}

/**
 * @brief Sends a ping to the ATtiny to reset its timeout counter.
 */
void keepAlive() {
  Wire.beginTransmission(SLAVE_ATTINY_ADDRESS);
  Wire.write(21); // Command 21: Keep alive / Ping
  Wire.endTransmission(true);
}

// #define DEBUGMIO // Uncomment to enable serial debugging

void setup() {
#ifdef DEBUGMIO
  delay(1000);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(9600);
#endif

  // Turn WiFi OFF immediately to save power during sensor reading
  wifiK::setWifiOff();
  Wire.begin(default_sda_pin, default_scl_pin);

  ErrorState check = NO_ERROR;
  SensorDataHandler sHandler;
  EepromDataHandler eHandler;

  sHandler.initialize(check);

  if (static_cast<uint8_t>(check) > 0) {
    helpMet::handleProgramError(check);
    return;
  }

  meteoData mdata;

#if defined DUMMY_RECORD
  // Dummy data insertion for testing purposes
  uint8_t NRECORD = 5;
  DEBUG_PRINT("Resetting memory...")
  nEEPROM::resetMemory();
  eHandler.initialize(true);
  DEBUG_PRINT("Inserting dummy records...");
  for (size_t i = 0; i < NRECORD; i++) {
    check = sHandler.getDummyRecord(&mdata);
    eHandler.appendRecordToEeprom(&mdata);
    delay(50);
  }
#else
  eHandler.initialize(false);
  check = sHandler.getRecord(&mdata);
#endif

  if (static_cast<uint8_t>(check) > 0) {
    helpMet::handleProgramError(check);
    return;
  }

  // Connect to WiFi
  wifiK::connectWifi(check);

  if (static_cast<uint8_t>(check) > 0) {
    helpMet::handleProgramError(check);
    eHandler.appendRecordToEeprom(
        &mdata); // Save data for later if connection fails
    return;
  }
  delay(10);

  // Connect to MQTT broker
  wifiK::connectMQTT(check);

  if (static_cast<uint8_t>(check) > 0) {
    helpMet::handleProgramError(check);
    eHandler.appendRecordToEeprom(&mdata); // Save data for later if MQTT fails
    return;
  }

  wifiK::smartDelay(100);
  uint16_t storedrecords = eHandler.storedRecords();

  if (storedrecords == 0) {
    // No stored records, send the current one immediately
    DEBUG_PRINT("Sending single record.");
    uint8_t l_buffer[RECORD_SIZE];
    helpMet::bufferizeData(&mdata, l_buffer);

    Eeprom_job ejob = NO_JOB;
    bool result = wifiK::sendWithRetry(l_buffer, RECORD_SIZE, ejob);

    if (!result) {
      eHandler.appendRecordToEeprom(&mdata);
      DEBUG_PRINT("MQTT transmission failed, storing data to EEPROM.");
    }
  } else {
    // Multiple records stored, send everything in a block
    DEBUG_PRINT("Sending stored records...");
    eHandler.appendRecordToEeprom(&mdata);
    wifiK::smartDelay(100);
    eHandler.readAndSendStoredRecordFromEeprom();
  }
}

void loop() {
#ifdef DEBUGMIO
  Serial.println("Shutting down...");
  wifiK::transmitMQTT(logTopic, "Shutting down");
#endif

  wifiK::smartDelay(100);
  wifiK::setWifiOff();
  delay(100);
  shutDownNow();
}
