#include <Checkvoltage.h>
Checkvoltage cv;
//#include <TinyDebugSerial.h>
// Get this from https://github.com/rambo/TinyWire
#include "TinyWireS.h"
//TinyDebugSerial mySerial= TinyDebugSerial();
#define I2C_SLAVE_ADDRESS 0x02 // the 7-bit address (remember to change this when adapting this example)
// I2C
//attiny pin 0 = not(OC1A) = PORTB <- _BV(0) = SOIC pin 5 (I2C SDA, PWM)
//attiny pin 2 =           = PORTB <- _BV(2) = SOIC pin 7 (I2C SCL, Analog 1)
// The default buffer size, Can't recall the scope of defines right now
#ifndef TWI_RX_BUFFER_SIZE
#define TWI_RX_BUFFER_SIZE ( 16 )
#endif
int pinLed = 1;
int watch_dog_counter = 2;
volatile byte reg_position = 0;
const byte reg_size = sizeof(cv.dati); //dati son il valore della tensione vedi checkvoltage.ino
/*volatile uint8_t i2c_regs[] =
{
    0xDE,
    0xAD,
    0xBE,
    0xEF,
};*/
void setup(){
  pinMode(pinLed,OUTPUT);
  setup_watchdog(9); // approximately 8 seconds sleep
  TinyWireS.begin(I2C_SLAVE_ADDRESS);
  TinyWireS.onReceive(receiveEvent);
  TinyWireS.onRequest(requestEvent);
}
void loop(){
  if (watch_dog_counter>=2) {  // 90*8 sec =13 min : wait for timed out watchdog / flag is set when a watchdog timeout occurs
    watch_dog_counter=0;        // reset flag
    powerESP();
  }
  TinyWireS_stop_check();
}

void requestEvent()
{
  TinyWireS.send(cv.dati[reg_position]);
  // Increment the reg position on each read, and loop back to zero
  reg_position++;
  if (reg_position >= reg_size)
  {
    reg_position = 0;
  }
}
void receiveEvent(uint8_t howMany)
{
//powerDownEsp();
    if (howMany < 1)
    {
        // Sanity-check
        return;
    }
    if (howMany > TWI_RX_BUFFER_SIZE)
    {
        // Also insane number
        return;
    }
    reg_position = TinyWireS.receive();
    if(reg_position == 20) powerDownEsp();
}

void powerESP()
{
  digitalWrite(pinLed,HIGH);  // let led blink -> esp-01 power on
  tws_delay(150);
cv.begin();
  cv.readVcc();

}
void powerDownEsp(){
  tws_delay(50);
  digitalWrite(pinLed,LOW);   // esp-01  power off
  pinMode(pinLed,INPUT); // set all used port to intput to save power
  system_sleep();
  pinMode(pinLed,OUTPUT); // set all ports into state before sleep
}
