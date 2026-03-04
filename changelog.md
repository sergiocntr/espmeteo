# Changelog - ESPmeteo Project

All notable changes to this project will be documented in this file.

## [1.2] - 2026-03-04
### Added
- **Intelligent I2C Timeout (ATtiny85)**: The power management timer is now dynamically refreshed on any I2C activity (receive or request), preventing premature shutdowns during busy periods.
- **Keep-Alive Pulse (ESP-01)**: Implemented a `keepAlive()` function in the main controller to signal the ATtiny to stay powered during long backlogged data uploads.
- **Fail-safe Efficiency**: Reduced default inactivity timeout on ATtiny from 25s to 15s to optimize battery usage when the controller is truly idle.

### Changed
- Refactored I2C communication calls to use centralized constants (`SLAVE_ATTINY_ADDRESS`) for better maintainability.
- Improved circular buffer transmission loop in `c_circBuff.h` to include periodic heartbeats for the power manager.

## [1.1] - 2026-02-13
### Added
- Professional ASCII headers to all main files.
- Automated error handling system with `errorHandler.py`.
- English documentation and updated README files for GitHub readiness.
- Sensitive configuration template (`config.h.example`) and robust `.gitignore`.

### Changed
- Standardized Project structure for PlatformIO compatibility.
- Translated all internal comments from Italian to English.
- Refactored BME280 sensor logic and circular buffer management for better stability.
