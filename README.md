# ESPmeteo: Solar-Powered Smart Weather Station

A robust, ultra-low-power weather station based on ESP-01 and ATtiny85.

## Overview
This system is designed for long-term outdoor deployment. It features a solar-powered architecture where an ATtiny85 acts as a power manager for the ESP-01, waking it up only when necessary to perform measurements and transmit data via MQTT.

[**Live Dashboard**](http://www.developteamgold.altervista.org/meteofeletto/swpi_dashboard.1.php)

## Key Features
- **Ultra-low power**: ATtiny85 duty-cycles the ESP-01 to preserve battery.
- **Offline Data Logging**: If WiFi or MQTT is unavailable, data is cached on an external 24LC32/256 EEPROM.
- **Smart Sync**: Automatically uploads all cached records once the connection is restored.
- **Solar Charged**: Optimized for solar-panel-powered hardware configurations.

## System Architecture

### 1. ESP-01 (Main Application)
- **Sensing**: BME280 for Temperature, Humidity, and Pressure.
- **Connectivity**: WiFi + MQTT.
- **Persistence**: Hybrid storage logic (Real-time MQTT + EEPROM circular buffer cache).

### 2. ATtiny85 (Power Manager)
- **Control**: Manages the main power rail of the ESP-01.
- **Timing**: High-efficiency timing using the internal Watchdog Timer (~13 min cycles).
- **Telemetry**: Monitors system voltage and reports it back to the ESP-01 via I2C.

## Project Structure
- `/ESPmeteo`: Firmware for the ESP-01 (ESP8266).
- `/attinySavePower`: Firmware for the ATtiny85 power manager.

## Installation
Both projects are configured for **PlatformIO**. 

1. Open the project folder in VSCode with PlatformIO installed.
2. Select the appropriate environment (`esp01_1m` or `attiny`).
3. Build and upload.
