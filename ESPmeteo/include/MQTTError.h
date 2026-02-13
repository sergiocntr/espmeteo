#include "debugutils.h"

#ifndef MQTTERROR_H
#define MQTTERROR_H

enum class MQTTError {
    SUCCESS,
    CONNECTION_FAILED,
    PUBLISH_FAILED,
    SUBSCRIBE_FAILED,
    ACK_FAILED,
    ACK_OK,
    END_OK
};

#ifdef DEBUGMIO
const char* MQTTError_messages[] = {
    "MQTT connection OK!",
    "MQTT connection failed.",
    "Message publication failed.",
    "Topic subscription failed.",
    "ACK not received within timeout.",
    "Partial data sent and confirmed successfully!",
    "Total data sent and confirmed successfully!"
};
const char* MQTTError_className = "MQTTError";
#endif

#endif // MQTTERROR_H
