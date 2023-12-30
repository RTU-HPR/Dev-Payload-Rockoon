#pragma once
#include <ms5611.h>
#include <Wire.h>

// PINS payload v2  (if not used just comment it out don't delete)
int _SDA = 14;
int _SCL = 15;
TwoWire *_WIRE = &Wire1;
int MS5611_ADDRESS = 0x77; // could be 0x76

MS5611 ms5611(MS5611_ADDRESS);
void start()
{
    _WIRE->setSCL(_SCL);
    _WIRE->setSDA(_SDA);

    if (ms5611.begin(_WIRE) == true)
    {
        Serial.println("MS5611 found.");
    }
    else
    {
        Serial.println("MS5611 not found. halt.");
        while (1)
            ;
    }

    while (true)
    {
        ms5611.read(); // note no error checking => "optimistic".
        Serial.print("T:\t");
        Serial.print(ms5611.getTemperature(), 2);
        Serial.print("\tP:\t");
        Serial.print(ms5611.getPressure(), 2);
        Serial.println();
        delay(1000);
    }
}
