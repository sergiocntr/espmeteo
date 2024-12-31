// eeprommio.h
#pragma once
byte writeStructEEPROM(unsigned int addr)
{
	byte err;
	Wire.beginTransmission(EEPROM_I2C_ADDRESS);
	Wire.write((byte)(addr >> 8));	 // high order byte
	Wire.write((byte)(addr & 0xFF)); // low-order byte
	I2C_writeAnything(currmet);
	err = Wire.endTransmission();
	delay(200); //
	return err; // cannot write to device
}
byte readStructEEPROM(unsigned int addr)
{
	byte err;
	Wire.beginTransmission(EEPROM_I2C_ADDRESS);
	Wire.write((byte)(addr >> 8));	 // high order byte
	Wire.write((byte)(addr & 0xFF)); // low-order byte
	err = Wire.endTransmission();
	// initiate blocking read into internal buffer
	Wire.requestFrom(EEPROM_I2C_ADDRESS, sizeof(met));

	I2C_readAnything(met);

	return err;
}

