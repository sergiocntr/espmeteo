## GESTIONE MEMORIA 24LC256 CON BUFFER CIRCOLARE
- La memoria ha 512 pagine da 64 byte
- I record sono di 16 byte -> 4 record per pagina

- L'ultima pagina e' riservata per il buffer circolare
- Il buffer circolare e' di 6 byte

- queste le definizioni delel struct e delle variabili globali: 
```cpp
struct meteoData{
	float humidityBMP; 			//4 byte
	float temperatureBMP; 		//4 byte
	float externalPressure; 	//4 byte
  	uint16_t battery;			//2 byte
	uint8_t counter;       		// 1 byte: counter record 
	uint8_t checksum;   		// 1 byte: checksum per la validità dei dati
};
meteoData currmet;

struct eeCircRecord {
    uint8_t wipeFlag = 0xFF;        // Default: 0xFF
    uint8_t flag = 0xAA;            // Default: 0xAA
    uint16_t pageAddress = 0;       // Default: 0
    uint16_t recordCount = 0;       // Default: 0
};
eeCircRecord eBuffer;
uint16_t eBufferAddress;
```

### Logica di funzionamento
1. Al fine di preservare batteria e cicli di scrittura sulla EEPROM ,all avvio viene chiamato 
```cpp
readLastPageAddressAndRecordCount()
```
il quale si incarica di popolare la struct **eBuffer** e **eBufferAddress**

2. Il programma a questo punto interroga il sensore meteo e popola la struct **currmet**
3. Se **eeCircRecord.recordCount > 0** aggiunge il record all EEPROM
```cpp
void appendRecordToPage(meteoData newRecord)
```
4. Tentativo di connessione al WIFI
- Se il tentativo fallisce spegne.
- Se il tentativo ha successo manda **currmet** se **eeCircRecord.recordCount = 0** altrimenti recupera tutti i recordo con 
```cpp
void appendRecordToPage(meteoData newRecord)
```

il quale si incarica di scrivere nella EEPROM il nuovo dato raccolto in **currmet** e di aggiornare **eeCircRecord** aumentando il record count di 1. Il processo finisce.
- Se il tentativo di connessione ha successo abbiamo due opzioni ,se **eBuffer.recordCount > 0** devo leggere la memoria
