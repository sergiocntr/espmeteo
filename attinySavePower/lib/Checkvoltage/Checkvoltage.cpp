/*
*  MANDA IL VALORE DELLA TENSIONE LETTA CON ATTINY
*
*/
#include "Arduino.h"
#include "TinyWireS.h"
#include "Checkvoltage.h"
Checkvoltage::Checkvoltage()
{

}
void Checkvoltage::begin(){
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif
}
void Checkvoltage::readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference


  tws_delay(5); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring
  volt = ADC; //Needs to be 16 bit long ! This only reads the two registers in one command
  volt = 1126400L / volt; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  //splitto in due byte
  dati[0]=volt & 0xff;
  dati[1]=(volt >> 8);
  ADMUX&=~(REFS0);
  ADMUX&=~(REFS1);

}
