import os

# Definizione degli errori
ERROR_DEFINITIONS = {
    "EEPROMError": {
        "SUCCESS": "Operazione completata con successo.",
        "FATAL_ERROR": "Errore persistente nella lettura dei dati dalla EEPROM!",
        "OUT_OF_RANGE": "Indirizzo fuori dai limiti.",
        "EE_FULL": "Errore: Memoria EEPROM DATI piena!",
        "I2C_TRANSMISSION_FAILED": "Errore di trasmissione I²C."

    },
    "MQTTError": {
        "SUCCESS": "Connessione MQTT OK!",
        "CONNECTION_FAILED": "Connessione MQTT fallita.",
        "PUBLISH_FAILED": "Pubblicazione del messaggio fallita.",
        "SUBSCRIBE_FAILED": "Iscrizione al topic fallita.",
        "ACK_FAILED": "ACK non ricevuto ento il timeout",
        "ACK_OK": "Dati parziali inviati e confermati con successo!",
        "END_OK": "Dati totali inviati e confermati con successo!"
    },
    "WIFIError": {
        "SUCCESS": "Connessione WIFI ok.",
        "CONNECTION_FAILED": "Connessione WIFI fallita."
        
    },
    "SENSORError": {
        "BME280_SUCCESS": "BME280 ok.",
        "BME280_CONNECTION_FAILED": "Il sensore BME280 non risponde.",
        "BME280_DATA_ERROR": "Dati non validi letti dal sensore BME280."
    # ERROR_PRINT(SENSORError_className, SENSORError, SENSORError::BME280_DATA_ERROR);
    # sprintf(helpMet::str, "Spedisco multipli record.");
	# DEBUG_PRINT(helpMet::str);    
    },"WIREError": {
        "SUCCESS": "Transfer ok!.",
        "BYTE_NUM_ERROR": "Numero di byte richiesti non corrisponde.",
        "DATA_ERROR": "Dati incompleti ricevuti"
        
    },"CIRBUFFError": {
        "SUCCESS": "Transfer ok!.",
        "CIRCREC_NOT_FOUND": "Errore nel recuperare circRecordAddress.",
        "CIRCREC_NOT_BUILD": "Errore durante l'inizializzazione del buffer circolare."
        
    }
}

# Directory di output
OUTPUT_DIR = "include"

# Template per il file header
HEADER_TEMPLATE = """\
#include "debugutils.h"
#ifndef {enum_name_upper}_H
#define {enum_name_upper}_H

enum class {enum_name} {{
{enum_values}
}};

#ifdef DEBUGMIO
const char* {enum_name}_messages[] = {{
{enum_messages}
}};
const char* {enum_name}_className = "{enum_name}";  // Nome della classe
#endif

#endif // {enum_name_upper}_H
"""

# Generazione dei file
os.makedirs(OUTPUT_DIR, exist_ok=True)
include_string = ''
for enum_name, errors in ERROR_DEFINITIONS.items():
    # Costruzione enum e messaggi
    enum_values = ",\n".join(f"    {key}" for key in errors.keys())
    enum_messages = ",\n".join(f'    "{value}"' for value in errors.values())
    enum_name_upper = enum_name.upper()

    # Creazione contenuto file header
    header_content = HEADER_TEMPLATE.format(
        enum_name=enum_name,
        enum_name_upper=enum_name_upper,
        enum_values=enum_values,
        enum_messages=enum_messages
    )

    include_string += f"#include <{enum_name}.h> \n" 

    # Scrittura su file
    output_file = os.path.join(OUTPUT_DIR, f"{enum_name}.h")
    with open(output_file, "w") as f:
        f.write(header_content)
output_file = os.path.join(OUTPUT_DIR, "m_errors.h")
with open(output_file, "w") as f:
        f.write(include_string)






print(f"File generati nella directory: {OUTPUT_DIR}")
