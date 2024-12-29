// eeprommio.h
#pragma once
byte writeStructEEPROM(unsigned int addr)
{
	byte err;
	Wire.beginTransmission(SLAVE_ADDRESS);
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
	Wire.beginTransmission(SLAVE_ADDRESS);
	Wire.write((byte)(addr >> 8));	 // high order byte
	Wire.write((byte)(addr & 0xFF)); // low-order byte
	err = Wire.endTransmission();
	// initiate blocking read into internal buffer
	Wire.requestFrom(SLAVE_ADDRESS, sizeof(met));

	I2C_readAnything(met);

	return err;
}
void writeEEPROM(uint16_t eeaddress, uint8_t data)
{
	Wire.beginTransmission(SLAVE_ADDRESS);
	Wire.write((int)(eeaddress >> 8));	 // MSB
	Wire.write((int)(eeaddress & 0xFF)); // LSB
	Wire.write(data);
	Wire.endTransmission();
	delay(10);
}
uint8_t readEEPROM(uint16_t eeaddress)
{
	uint8_t rdata = 0xFF;
	Wire.beginTransmission(SLAVE_ADDRESS);
	Wire.write((int)(eeaddress >> 8));	 // MSB
	Wire.write((int)(eeaddress & 0xFF)); // LSB
	Wire.endTransmission();
	Wire.requestFrom(SLAVE_ADDRESS, 1);
	if (Wire.available())
		rdata = Wire.read();
	return rdata;
}
void storeData(uint8_t nrRecords)
{
	const uint8_t maxRecords = 250; // Limite massimo di 250 record
	uint16_t availAddress;

	if (nrRecords >= maxRecords)
	{
		// Se abbiamo raggiunto il limite massimo, iniziamo a sovrascrivere
		nrRecords = 0; // Resetta l'indice per sovrascrivere i dati più vecchi
	}

	availAddress = 32 * nrRecords;	 // MeteoData è 24 byte, usiamo pagine di 32 byte
	writeStructEEPROM(availAddress); // Scrive la struct nella EEPROM I2C
	nrRecords++;					 // Incrementa il numero di record

	writeEEPROM(nValuesAddr, nrRecords); // Aggiorna il numero di record memorizzati
}
uint16_t recuperoPosMemoria()
{
	// PER AUMENTARE LA VITA DELLA MEMORIA METTO UN BUFFER CIRCOLARE
	// DOVE MEMORIZZARE IL NUMERO DI RECORD SALVATI
	// OGNI VOLTA CHE SI CANCELLA LA POSIZIONE DI MEMORIA DI SCRIVE AA INVECE DI ZERO
	// E SI USA LA CELLA DOPO
	const uint16_t nValuesAddr = 0x0FE0; // indirizzo memoria di partenza
	const uint8_t defaultValue = 0xAA;	 // valore di riferimento ,la cella attuale di memoria è quella dopo
	for (uint16_t i = nValuesAddr; i < 0x0FFE; i++)
	{
		uint8_t myval = readEEPROM(i);
		if (myval == defaultValue)
		{

			return i;
		}
	}
	writeEEPROM(nValuesAddr, 0XAA);
	return nValuesAddr;
}
