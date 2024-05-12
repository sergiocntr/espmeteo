#pragma once
#include <Arduino.h>
#include "ESPmeteo.h"
void blinkLed(uint8_t volte, bool l)
{
	uint16_t tempo;

	if (l)
	{
		tempo = 250;
	}
	else
	{
		tempo = 500;
	}
	for (uint8_t i = 0; i < volte; i++)
	{
		digitalWrite(LED_BUILTIN, LOW);
		delay(tempo);
		digitalWrite(LED_BUILTIN, HIGH);
		delay(tempo);
	}
}
void faiSampleDummy()
{
	for (size_t i = 0; i < 4; i++)
	{
		requestSensorsValues();
		delay(10);
		storeData(i);
		delay(100);
		blinkLed(1, true);
		delay(2000);
	}
}