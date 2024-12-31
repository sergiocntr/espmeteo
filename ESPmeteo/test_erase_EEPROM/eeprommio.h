// eeprommio.h
#pragma once
void writeEEPROM(uint16_t eeaddress, uint8_t data)
{
    Wire.beginTransmission(SLAVE_ADDRESS);
    Wire.write((int)(eeaddress >> 8));   // MSB
    Wire.write((int)(eeaddress & 0xFF)); // LSB
    Wire.write(data);
    Wire.endTransmission();
    delay(10);
}
uint8_t readEEPROM(uint16_t eeaddress)
{
    uint8_t rdata = 0xFF;
    Wire.beginTransmission(SLAVE_ADDRESS);
    Wire.write((int)(eeaddress >> 8));   // MSB
    Wire.write((int)(eeaddress & 0xFF)); // LSB
    Wire.endTransmission();
    Wire.requestFrom(SLAVE_ADDRESS, 1);
    if (Wire.available())
        rdata = Wire.read();
    return rdata;
}

void resetCircBuffer()
{
    const uint16_t nValuesAddr = 0x0FE0; // Indirizzo memoria di partenza
    const uint8_t defaultValue = 0xFF;   // Valore predefinito per il reset

    DEBUG_PRINT("resetCircBuffer memoria ....");

    // Scrittura dei valori nell'EEPROM
    for (uint16_t i = 0; i < 64; i += 16)
    { // Scrittura a blocchi di 16 byte (esempio)
        Wire.beginTransmission(SLAVE_ADDRESS);

        // Imposta l'indirizzo corrente
        Wire.write((uint8_t)((nValuesAddr + i) >> 8));   // Byte alto dell'indirizzo
        Wire.write((uint8_t)((nValuesAddr + i) & 0xFF)); // Byte basso dell'indirizzo

        // Scrive 16 byte per ciclo
        for (uint8_t j = 0; j < 16; j++)
        {
            Wire.write(defaultValue);
        }

        // Conclude la trasmissione e verifica eventuali errori
        uint8_t result = Wire.endTransmission();
        if (result != 0)
        {
            DEBUG_PRINT("Errore nella scrittura I2C");
            return; // Esce se c'è un errore
        }

        delay(50); // Attesa per completare la scrittura
    }

    // Lettura e verifica dei valori scritti
    Wire.beginTransmission(SLAVE_ADDRESS);
    Wire.write((uint8_t)(nValuesAddr >> 8));   // Byte alto dell'indirizzo
    Wire.write((uint8_t)(nValuesAddr & 0xFF)); // Byte basso dell'indirizzo
    Wire.endTransmission();

    Wire.requestFrom(SLAVE_ADDRESS, 64); // Richiede 64 byte

    DEBUG_PRINT("Lettura valori:");
    for (uint16_t i = 0; i < 64; i++)
    {
        if (Wire.available())
        {
            uint8_t value = Wire.read();
            Serial.print("0x");
            Serial.print(value, HEX);
            Serial.print(" ");
        }
        else
        {
            DEBUG_PRINT("Errore: dati non disponibili");
            Serial.print("nrByte: ");
            Serial.print(i);
            break;
        }
    }
    Serial.println(); // Va a capo alla fine della stampa
}

void resetMemory()
{
    const uint16_t nValuesAddr = 0x0000; // Indirizzo memoria di partenza
    const uint8_t defaultValue = 0xCC;   // Valore predefinito per il reset
    const uint16_t byteToErase = 2048; // Indirizzo memoria di partenza
    const int transferSize = 32;    // Valore predefinito per il reset

    DEBUG_PRINT("reset Memoria ....");

    // Scrittura dei valori nell'EEPROM
    for (uint16_t i = 0; i < byteToErase; i += transferSize)
    { // Scrittura a blocchi di 16 byte (esempio)
        Wire.beginTransmission(SLAVE_ADDRESS);

        // Imposta l'indirizzo corrente
        Wire.write((uint8_t)((nValuesAddr + i) >> 8));   // Byte alto dell'indirizzo
        Wire.write((uint8_t)((nValuesAddr + i) & 0xFF)); // Byte basso dell'indirizzo

        // Scrive 16 byte per ciclo
        for (uint8_t j = 0; j < transferSize; j++)
        {
            Wire.write(defaultValue);
        }

        // Conclude la trasmissione e verifica eventuali errori
        uint8_t result = Wire.endTransmission();
        if (result != 0)
        {
            DEBUG_PRINT("Errore nella scrittura I2C");
            return; // Esce se c'è un errore
        }

        delay(50); // Attesa per completare la scrittura
    }
    // Scrittura dei valori nell'EEPROM
    for (uint16_t i = 0; i < byteToErase; i += transferSize)
    { // Scrittura a blocchi di 16 byte (esempio)
        Wire.beginTransmission(SLAVE_ADDRESS);
        Wire.write((uint8_t)(nValuesAddr >> 8));   // Byte alto dell'indirizzo
        Wire.write((uint8_t)(nValuesAddr & 0xFF)); // Byte basso dell'indirizzo
        Wire.endTransmission();

        Wire.requestFrom(SLAVE_ADDRESS, transferSize); // Richiede 64 byte

        
        DEBUG_PRINT("Lettura valori:");
        for (uint16_t i = 0; i < transferSize; i++)
        {
            if (Wire.available())
            {
                uint8_t value = Wire.read();
                Serial.print("0x");
                Serial.print(value, HEX);
                Serial.print(" ");
            }
            else
            {
                DEBUG_PRINT("Errore: dati non disponibili");
                Serial.print("nrByte: ");
                Serial.print(i);
                break;
            }
        }
        Serial.println(); // Va a capo alla fine della stampa
    }
    
}
