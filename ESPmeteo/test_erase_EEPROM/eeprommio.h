// eeprommio.h
#pragma once
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

void resetMemory()
{
	// PER AUMENTARE LA VITA DELLA MEMORIA METTO UN BUFFER CIRCOLARE
	// DOVE MEMORIZZARE IL NUMERO DI RECORD SALVATI
	// OGNI VOLTA CHE SI CANCELLA LA POSIZIONE DI MEMORIA SI SCRIVE 0xAA INVECE DI ZERO
	// E SI USA LA CELLA DOPO
	const uint16_t nValuesAddr = 0x0FE0; // Indirizzo memoria di partenza
	const uint8_t defaultValue = 0xFF;    // Valore di riferimento; la cella attuale di memoria è quella dopo

	DEBUG_PRINT("Verifica memoria");

	for (uint16_t i = nValuesAddr; i < 0x0FFE; i++)
	{
		// Scrive il valore di default nella memoria
		writeEEPROM(i, defaultValue);
		
		// Legge il valore per verificare che sia stato scritto correttamente
		uint8_t myval = readEEPROM(i);
		if (myval != defaultValue)
		{
			DEBUG_PRINT("Errore eeprom all'indirizzo ", i);
		}
	}

	DEBUG_PRINT("Fine verifica");
}
