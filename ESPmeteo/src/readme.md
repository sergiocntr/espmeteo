# ESPmeteo Memory & Logic Management

This document describes the EEPROM storage strategy and the main program logic for the ESP-01 Weather Station.

## EEPROM Storage (24LC32/24LC256)
The station uses an external I2C EEPROM to store data when an internet connection is unavailable.

- **Geometry**: The memory is managed in 64-byte pages.
- **Records**: Each weather record is 16 bytes, allowing for 4 records per page.
- **Circular Buffer**: The last page is reserved for a circular pointer (6 bytes) that tracks the last written address and total record count.

### Data Structures

```cpp
struct meteoData {
    int16_t humidityBMP;       // Humidity (fixed point x128)
    int16_t temperatureBMP;    // Temperature (fixed point x128)
    int16_t externalPressure;  // Pressure (fixed point x32)
    uint16_t battery;          // Battery voltage in mV
    uint8_t counter;           // Record sequence counter
    uint8_t checksum;          // Data integrity byte
    uint8_t padding[5];        // Alignment padding
    uint32_t deviceID;         // Unique device identifier
};

struct eeCircRecord {
    uint8_t wipeFlag;          // Debug/Wipe flag
    uint8_t flag;              // Sync flag (0xAA)
    uint16_t pageAddress;      // Current page address in EEPROM
    uint16_t recordCount;      // Number of stored records
};
```

## Operation Logic

1. **Initialization**: On boot, the station reads the last page of the EEPROM to recover the state of the circular buffer (`eBuffer`).
2. **Sensing**: Atmospheric data is sampled from the BME280 sensor, and battery voltage is requested from the ATtiny85 via I2C.
3. **Connectivity Check**:
    - The station attempts to connect to WiFi and the MQTT broker.
4. **Data Transmission**:
    - **Success**: If connected, it sends the current record. If there are stored records from previous failed attempts, it transmits the entire backlog.
    - **Failure**: If connection fails, the current record is appended to the EEPROM for future transmission.
5. **Sleep**: Once the task is complete , the ESP-01 sends a shutdown command (I2C `20`) to the ATtiny85 and enters deep sleep.
    - The ATtiny85 has a timeout of 15 seconds. If it doesn't receive any I2C communication from the ESP-01 for 15 seconds, it will shut down the ESP-01.
    - The ESP-01 sends a keep-alive pulse (I2C `21`) to the ATtiny85 every 500ms to reset the timeout counter.