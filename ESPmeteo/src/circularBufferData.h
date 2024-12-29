#include <Wire.h>

#define EEPROM_PAGE_SIZE 64
#define RECORD_SIZE 16
#define BUFFER_CIRCULAR_START 0x0FE0
#define DEFAULT_FLAG 0xAA

uint16_t readLastPageAddressAndRecordCount(uint16_t &recordCount) {
    for (uint16_t addr = BUFFER_CIRCULAR_START; addr < BUFFER_CIRCULAR_START + EEPROM_PAGE_SIZE; addr += 3) {
        uint8_t flag = readEEPROM(addr);
        if (flag == DEFAULT_FLAG) {
            recordCount = (readEEPROM(addr + 1) << 8) | readEEPROM(addr + 2);
            return addr;
        }
    }
    return BUFFER_CIRCULAR_START; // Default to start if no valid flag found
}

bool isPageFull(uint16_t recordCount) {
    return (recordCount % (EEPROM_PAGE_SIZE / RECORD_SIZE) == 0);
}

void writePage(uint16_t pageAddr, const uint8_t *data, uint16_t size) {
    Wire.beginTransmission(SLAVE_ADDRESS);
    Wire.write((byte)(pageAddr >> 8));
    Wire.write((byte)(pageAddr & 0xFF));
    for (uint16_t i = 0; i < size; i++) {
        Wire.write(data[i]);
    }
    Wire.endTransmission();
    delay(5); // Ensure the write is committed
}

void appendRecordToPage(uint16_t pageAddr, const uint8_t *newRecord, uint16_t recordCount) {
    uint8_t pageData[EEPROM_PAGE_SIZE];
    
    // Read current page data
    for (uint16_t i = 0; i < EEPROM_PAGE_SIZE; i++) {
        pageData[i] = readEEPROM(pageAddr + i);
    }

    // Append new record
    uint16_t offset = (recordCount % (EEPROM_PAGE_SIZE / RECORD_SIZE)) * RECORD_SIZE;
    memcpy(&pageData[offset], newRecord, RECORD_SIZE);

    // Write back updated page
    writePage(pageAddr, pageData, EEPROM_PAGE_SIZE);
}

void updateCircularBuffer(uint16_t bufferAddr, uint16_t recordCount) {
    writeEEPROM(bufferAddr, DEFAULT_FLAG);
    writeEEPROM(bufferAddr + 1, (recordCount >> 8) & 0xFF);
    writeEEPROM(bufferAddr + 2, recordCount & 0xFF);
}

void storeNewRecord(const uint8_t *newRecord) {
    uint16_t recordCount = 0;
    uint16_t bufferAddr = readLastPageAddressAndRecordCount(recordCount);

    uint16_t pageAddr = EEPROM_PAGE_SIZE * (recordCount / (EEPROM_PAGE_SIZE / RECORD_SIZE));

    if (isPageFull(recordCount)) {
        // Start new page
        pageAddr += EEPROM_PAGE_SIZE;
        writePage(pageAddr, newRecord, RECORD_SIZE);
    } else {
        // Append to existing page
        appendRecordToPage(pageAddr, newRecord, recordCount);
    }

    // Update circular buffer
    recordCount++;
    bufferAddr = (bufferAddr + 3) % (BUFFER_CIRCULAR_START + EEPROM_PAGE_SIZE);
    updateCircularBuffer(bufferAddr, recordCount);
}

// Helper functions for EEPROM read/write
uint8_t readEEPROM(uint16_t addr) {
    Wire.beginTransmission(SLAVE_ADDRESS);
    Wire.write((byte)(addr >> 8));
    Wire.write((byte)(addr & 0xFF));
    Wire.endTransmission();

    Wire.requestFrom(SLAVE_ADDRESS, 1);
    return Wire.available() ? Wire.read() : 0xFF;
}

void writeEEPROM(uint16_t addr, uint8_t value) {
    Wire.beginTransmission(SLAVE_ADDRESS);
    Wire.write((byte)(addr >> 8));
    Wire.write((byte)(addr & 0xFF));
    Wire.write(value);
    Wire.endTransmission();
    delay(5);
}
