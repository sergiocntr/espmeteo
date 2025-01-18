//impostazioni.h
#pragma once
const int default_sda_pin = 0;
const int default_scl_pin = 2;


// I2C eeprom stuff
const int SLAVE_ATTINY_ADDRESS = 2; //
const int EEPROM_I2C_ADDRESS = 0X50;     // classic I2C EEPROM address
struct meteoData{
	float humidityBMP; 			//4 byte
	float temperatureBMP; 		//4 byte
	float externalPressure; 	//4 byte
  	uint16_t battery;			//2 byte
	uint8_t counter;       		// 1 byte: nr of record
	uint8_t checksum;   		// 1 byte: checksum per la validità dei dati
};
struct eeCircRecord {
    uint8_t wipeFlag = 0xFF;        // Default: 0xFF
    uint8_t flag = 0xAA;            // Default: 0xAA
    uint16_t pageAddress = 0x0000;       // Default: 0
    uint16_t recordCount = 0;       // Default: 0
};
enum ErrorState {
    NO_ERROR = 0,             	// Nessun errore, tutto OK
    BME280_CONNECTION_FAILED = 1,         		// Errore nella lettura della sonda BME280
	BME280_DATA_ERROR = 2,
	ATTINY_BYTE_NUM_ERROR = 3,			// Errore nel Attiny 85
	ATTINY_DATA_ERROR = 4,			// Errore nel Attiny 85
    EEPROM_ERROR = 5,         	// Errore nell'accesso o scrittura sull'EEPROM
	WIFI_CONNECTION_ERROR = 6, 	// Errore nella connessione WIFI
    MQTT_CONNECTION_ERROR = 7, 	// Errore nella connessione MQTT
	MQTT_LOOP_ERROR = 8 	// Errore nel loop MQTT
};
enum AckState {// QUESTO E' SOLO LA GESTIONE DEGLI ACK
    NO_ACK = 0,		// NO ACK : NESSUNO HA GESTITO IL MESSAGGIO INOLTRATO ,COME SE MQTT FOSSE SPENTO O NESSUNO SOTTOSCRITTO AL TOPIC. IL MESSAGGIO DEVE ESSERE MEMORIZZATO IN EEPROM
    ACK = 1,        // E' ARRIVATO UN MESSAGGIO CORRETTO ,SI ASPETTANO GLI ALTRI MESSAGGI'
	FAILED = 2,		//  E' ARRIVATO UN MESSAGGIO CON CHECKSUM SBAGLIATO E VA RITRASMESSO
	END = 3,			// RICEVUTI TUTTI I MESSAGGI ,LA EEPROM PUO ESSERE RESETTATA
	ERROR = 4	//ERRORE NELLA EEPROM
};
enum Eeprom_job{
	NO_JOB = 0,
	CONTINUE = 1, //CONTINUAMO LA TRASMISSIONE
	UPDATE_EEPROM = 3, // LA TRASMISSIONE E' RIUSCITA , AVANZO IL RECORD CIRCOLARE
	ERASE_EEPROM = 4 //ERRORE FATALE , RESETTO LA EEPROM

};
