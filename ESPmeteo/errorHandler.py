import os

"""
errorHandler.py
Utility script to generate C++ header files for error handling.
Translates error definitions into enum classes and human-readable messages.
"""

# Error definitions with English messages
ERROR_DEFINITIONS = {
    "EEPROMError": {
        "SUCCESS": "Operation completed successfully.",
        "FATAL_ERROR": "Persistent error reading data from EEPROM!",
        "OUT_OF_RANGE": "Address out of range.",
        "EE_FULL": "Error: EEPROM data memory full!",
        "I2C_TRANSMISSION_FAILED": "I2C transmission error."
    },
    "MQTTError": {
        "SUCCESS": "MQTT connection OK!",
        "CONNECTION_FAILED": "MQTT connection failed.",
        "PUBLISH_FAILED": "Message publication failed.",
        "SUBSCRIBE_FAILED": "Topic subscription failed.",
        "ACK_FAILED": "ACK not received within timeout.",
        "ACK_OK": "Partial data sent and confirmed successfully!",
        "END_OK": "Total data sent and confirmed successfully!"
    },
    "WIFIError": {
        "SUCCESS": "WiFi connection OK.",
        "CONNECTION_FAILED": "WiFi connection failed."
    },
    "SENSORError": {
        "BME280_SUCCESS": "BME280 OK.",
        "BME280_CONNECTION_FAILED": "BME280 sensor not responding.",
        "BME280_DATA_ERROR": "Invalid data read from BME280 sensor."
    },
    "WIREError": {
        "SUCCESS": "Transfer OK!",
        "BYTE_NUM_ERROR": "Requested byte count mismatch.",
        "DATA_ERROR": "Incomplete data received."
    },
    "CIRBUFFError": {
        "SUCCESS": "Transfer OK!",
        "CIRCREC_NOT_FOUND": "Error retrieving circRecordAddress.",
        "CIRCREC_NOT_BUILD": "Error during circular buffer initialization."
    }
}

OUTPUT_DIR = "include"

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
const char* {enum_name}_className = "{enum_name}";
#endif

#endif // {enum_name_upper}_H
"""

def generate_headers():
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    include_string = '#pragma once\n\n'
    
    for enum_name, errors in ERROR_DEFINITIONS.items():
        enum_values = ",\n".join(f"    {key}" for key in errors.keys())
        enum_messages = ",\n".join(f'    "{value}"' for value in errors.values())
        enum_name_upper = enum_name.upper()

        header_content = HEADER_TEMPLATE.format(
            enum_name=enum_name,
            enum_name_upper=enum_name_upper,
            enum_values=enum_values,
            enum_messages=enum_messages
        )

        include_string += f'#include "{enum_name}.h"\n'

        output_file = os.path.join(OUTPUT_DIR, f"{enum_name}.h")
        with open(output_file, "w", encoding="utf-8") as f:
            f.write(header_content)
            
    # Generate the main errors file that includes all headers
    main_errors_file = os.path.join(OUTPUT_DIR, "m_errors.h")
    with open(main_errors_file, "w", encoding="utf-8") as f:
        f.write(include_string)

    print(f"Header files generated successfully in: {OUTPUT_DIR}")

if __name__ == "__main__":
    generate_headers()
