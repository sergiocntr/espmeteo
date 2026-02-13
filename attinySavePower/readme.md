# ATtiny85 Power Manager for ESP-01

This firmware manages the power supply of an ESP-01 module to optimize battery performance in remote weather stations.

## Features
- **I2C Communication**: Acts as an I2C Slave (address `0x02`).
- **Conditional Shutdown**: Powers off the ESP-01 upon receiving command `20` from the ESP-01 via I2C.
- **Watchdog Wakeup**: Periodically wakes up the ESP-01 (approx. every 13 minutes) to perform a measurement cycle.
- **Deep Sleep**: Utilizes the AVR `SLEEP_MODE_PWR_DOWN` to reduce consumption to micro-amps while waiting.
- **Battery Telemetry**: Measures and provides the system voltage to the ESP-01 via I2C.

## Hardware setup
- **Pin 1 (PB1)**: Controls the power (usually via a MOSFET or directly if current is low enough) of the ESP-01.
- **I2C Pins**: Standard ATtiny85 I2C pins for communication with the ESP-01.