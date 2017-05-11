// Date and time functions using just software, based on millis() & timer
// LOG Shower monitor 2
// Current date is set whenever this sketch is loaded
// The Real Time Clock uses a DS1307
// Shower LED will be on every other day
// To shift Shower day one day, press and hold Shift button about three seconds
//
// dayOfWeek starts with 0 = Sunday

#include <TinyWireM.h>
#include <TinyRTClib.h>
#define WIRE TinyWireM
RTC_DS1307 RTC;		// RTC is a DS1307 type

int day;
int dayPin[7];		// Array for day of the week LEDs
boolean shower;
boolean firstSun;	// firstSun is true the first time the
					// Arduino is accessed on Sunday, then it is false
DateTime now;		// now is a variable of DateTime type

// Digital pins
#define SunPin 13
#define MonPin 12
#define TuePin 11
#define WedPin 10
#define ThuPin 7
#define FriPin 9
#define SatPin 8
#define ShowerPin 6

#define shiftPin 2			// Used to shift shower day
#define poweronPin 3		// Turn on Arduino

#define showerAddress 0		// Address in EEPROM
#define firstSunAddress 1	// Address in EEPROM

// Prototypes
void i2c_eeprom_write_byte(int , unsigned int);
byte i2c_eeprom_read_byte(int , unsigned int);
boolean readEEPROM(unsigned int);		// read EEPROM byte
void writeEEPROM(unsigned int , byte);	// write EEPROM byte

void setup () {
//Serial.begin(57600);
	dayPin[0] = SunPin;
	dayPin[1] = MonPin;
	dayPin[2] = TuePin;
	dayPin[3] = WedPin;
	dayPin[4] = ThuPin;
	dayPin[5] = FriPin;
	dayPin[6] = SatPin;
	for (day = 0; day < 7; day++){
		pinMode(dayPin[day], OUTPUT);	// Set for output
	}
	pinMode(ShowerPin, OUTPUT);			// Set for output
    pinMode(shiftPin, INPUT_PULLUP);
	pinMode(poweronPin, OUTPUT);

    digitalWrite(poweronPin, HIGH); 	// latches poweron
	WIRE.begin();
    RTC.begin();


}

void loop () {
	shower = readEEPROM(showerAddress);		// get shower status from EEPROM
	firstSun = readEEPROM(firstSunAddress);	// get firstSun from EEPROM

	now = RTC.now();		// Reads the RTC time/date
/*	Serial.print(now.year(), DEC);
	Serial.print('/');
	Serial.print(now.month(), DEC);
	Serial.print('/');
	Serial.print(now.day(), DEC);
	Serial.print(' ');
	Serial.print(now.hour(), DEC);
	Serial.print(':');
	Serial.print(now.minute(), DEC);
	Serial.print(':');
	Serial.print(now.second(), DEC);
	Serial.println();
*/
	// Turn on correct LED
	for (day = 0; day < 7; day++){
		if ( day != now.dayOfWeek() ) digitalWrite(dayPin[day], LOW);	// Turn off LED
		else digitalWrite(dayPin[day], HIGH);							// Turn on correct LED
	}

	// Week is seven days so if Sunday, need to correct Shower day
	// This is tricky, if it's Sunday and first time Start is pushed
	// then toggle shower and set firstSun to false
	// If Saturday (6) make sure firstSun is true
	// This is done so looking the second time on a Sunday doesn't
	//   toggle shower again.
	if ( (now.dayOfWeek() == 0) && firstSun ){
		shower = !shower;								// toggle shower
		writeEEPROM(showerAddress, byte(shower));		// write shower
		firstSun = false;
		writeEEPROM(firstSunAddress, byte(firstSun));	// write firstSun

	}
	if (now.dayOfWeek() == 6){
		firstSun = true;
		writeEEPROM(firstSunAddress, byte(firstSun));	// write firstSun
	}
	// Use modulus to see if it's shower day
	if ( (now.dayOfWeek() % 2) == shower)	digitalWrite(ShowerPin, HIGH);		// Turn on LED
	else digitalWrite(ShowerPin, LOW);		// Turn off LED
	delay(2000);
	// if Shift button is pushed, toggle shower so shower day is shifted.
	if (digitalRead(shiftPin) == LOW){
		shower = !shower;								// toggle shower
		writeEEPROM(showerAddress, byte(shower));		// write shower
	}
	delay(2000);
	// Turn power off unless shiftPin is pressed
	if (digitalRead(shiftPin) == HIGH) digitalWrite(poweronPin, LOW); // poweron is off

}

//*****************************************
//
//      24Cxx I2C EEPROM code
//
//*****************************************
void i2c_eeprom_write_byte( int deviceaddress, unsigned int eeaddress, byte data ) {
    int rdata = data;
    WIRE.beginTransmission(deviceaddress);
    WIRE.write((int)(eeaddress >> 8)); // MSB
    WIRE.write((int)(eeaddress & 0xFF)); // LSB
    WIRE.write(rdata);
    WIRE.endTransmission();
}

byte i2c_eeprom_read_byte( int deviceaddress, unsigned int eeaddress ) {
    byte rdata = 0xFF;
    WIRE.beginTransmission(deviceaddress);
    WIRE.write((int)(eeaddress >> 8)); // MSB
    WIRE.write((int)(eeaddress & 0xFF)); // LSB
    WIRE.endTransmission();
    WIRE.requestFrom(deviceaddress,1);
    if (WIRE.available()) rdata = WIRE.read();
    return rdata;
}

// read EEPROM byte
boolean readEEPROM(unsigned int eeaddress){
	// 0x50 is the EEPROM address
	// The first address from the memory, 0 contains Shower
    if( i2c_eeprom_read_byte(0x50, eeaddress) == 0) return false;
	else return true;
}

// write EEPROM byte
void writeEEPROM(unsigned int eeaddress, byte data){
	// 0x50 is the EEPROM address
	// The first address from the memory, 0 contains Shower
    i2c_eeprom_write_byte( 0x50, eeaddress, data);
	delay(100); 		//add a small delay
}
