/**
 * **************************************************************************
 *
 *    █████╗ ████████╗████████╗██╗███╗   ██╗██╗   ██╗
 *   ██╔══██╗╚══██╔══╝╚══██╔══╝██║████╗  ██║╚██╗ ██╔╝
 *   ███████║   ██║      ██║   ██║██╔██╗ ██║ ╚████╔╝
 *   ██╔══██║   ██║      ██║   ██║██║╚██╗██║  ╚██╔╝
 *   ██║  ██║   ██║      ██║   ██║██║ ╚████║   ██║
 *   ╚═╝  ╚═╝   ╚═╝      ╚═╝   ╚═╝╚═╝  ╚═══╝   ╚═╝
 *
 *   ██████╗  ██████╗ ██╗    ██╗███████╗██████╗
 *   ██╔══██╗██╔═══██╗██║    ██║██╔════╝██╔══██╗
 *   ██████╔╝██║   ██║██║ █╗ ██║█████╗  ██████╔╝
 *   ██╔═══╝ ██║   ██║██║███╗██║██╔══╝  ██╔══██╗
 *   ██║     ╚██████╔╝╚███╔███╔╝███████╗██║  ██║
 *   ╚═╝      ╚═════╝  ╚══╝╚══╝ ╚══════╝╚═╝  ╚═╝
 *
 * **************************************************************************
 * @file        main.cpp
 * @project     ATtiny85 Power Manager for ESP-01
 * @brief       Controls power to an ESP-01 module. Monitors battery and
 *              communicates via I2C to optimize power consumption.
 * @author      Sergio
 * @version     1.2
 * **************************************************************************
 */

#include "TinyWireS.h"
#include <Arduino.h>
#include <Checkvoltage.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

// --- Configuration ---
#define I2C_SLAVE_ADDRESS 0x02
#ifndef TWI_RX_BUFFER_SIZE
#define TWI_RX_BUFFER_SIZE 16
#endif

// --- Macros for Register Bit Manipulation ---
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

// --- Global Variables ---
const int PIN_POWER_CONTROL = 1;      // Pin to control ESP-01 power
volatile int watch_dog_counter = 100; // Counter for watchdog timeouts
uint32_t last_active_time;
byte reg_position = 0;
Checkvoltage cv;

// --- Function Prototypes ---
void system_sleep();
void setup_watchdog(int ii);
void powerDownEsp();
void receiveEvent(uint8_t howMany);
void requestEvent();

/**
 * @brief Puts the ATtiny into the deepest sleep mode (Power-down).
 *        Execution resumes after the watchdog timer wakes up the MCU.
 */
void system_sleep() {
  cbi(ADCSRA, ADEN);                   // Disable ADC to save power
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // Set deepest sleep mode
  sleep_enable();
  sleep_mode(); // System enters sleep here

  // --- SLEEPING ---

  sleep_disable();   // Resume execution here
  sbi(ADCSRA, ADEN); // Re-enable ADC
}

/**
 * @brief Configures the Watchdog Timer with a specific timeout.
 * @param ii Scaling factor: 0=16ms, 1=32ms, ..., 9=8s
 */
void setup_watchdog(int ii) {
  byte bb;
  if (ii > 9)
    ii = 9;
  bb = ii & 7;
  if (ii > 7)
    bb |= (1 << 5);
  bb |= (1 << WDCE);

  MCUSR &= ~(1 << WDRF);
  WDTCR |= (1 << WDCE) | (1 << WDE); // Start timed sequence
  WDTCR = bb;                        // Set new timeout value
  WDTCR |= _BV(WDIE);                // Enable WDT interrupt
}

/**
 * @brief ISR for Watchdog Timer. Increment the counter on each timeout.
 */
ISR(WDT_vect) { watch_dog_counter++; }

/**
 * @brief Handles power-down sequence for the ESP-01.
 */
void powerDownEsp() {
  tws_delay(50);
  digitalWrite(PIN_POWER_CONTROL, LOW); // Turn off ESP-01
  pinMode(PIN_POWER_CONTROL, INPUT);    // Set to Hi-Z to avoid leakage
  system_sleep();
  setup(); // Reset state after waking up
}

/**
 * @brief I2C OnReceive handler.
 */
void receiveEvent(uint8_t howMany) {
  last_active_time = millis(); // Refresh timeout on any receive event
  if (howMany < 1 || howMany > TWI_RX_BUFFER_SIZE)
    return;

  reg_position = TinyWireS.receive();

  // Command 20: Explicit shutdown requested by ESP-01
  if (reg_position == 20) {
    powerDownEsp();
  }
}

/**
 * @brief I2C OnRequest handler. Sends battery voltage data.
 */
void requestEvent() {
  last_active_time = millis(); // Refresh timeout on any request event
  TinyWireS.send(cv.dati[0]);
  TinyWireS.send(cv.dati[1]);
}

void setup() {
  pinMode(PIN_POWER_CONTROL, OUTPUT);

  cv.begin();
  cv.readVcc();

  setup_watchdog(9); // Setup for ~8s intervals
  reg_position = 0;

  TinyWireS.begin(I2C_SLAVE_ADDRESS);
  TinyWireS.onReceive(receiveEvent);
  TinyWireS.onRequest(requestEvent);

  last_active_time = millis();
}

void loop() {
  // Keep ESP-01 active for a maximum of 15 seconds (failsafe)
  // This timer is refreshed by any I2C communication from the ESP-01.
  while ((millis() - last_active_time) < 15000) {
    if (watch_dog_counter >= 100) {
      watch_dog_counter = 0;
      digitalWrite(PIN_POWER_CONTROL, HIGH); // Pulse or keep alive
    }

    // Brief delay to allow I2C processing
    tws_delay(500);
  }

  // Timeout reached, power down
  powerDownEsp();
}
