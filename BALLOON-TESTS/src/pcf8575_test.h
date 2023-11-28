#pragma once
#include <SPI.h>
#include <Wire.h>
#include <PCF8575.h>

///!!!! THe LIBRARY WAS MOTIFIED TO NOT CALL SET SCL AND SET SDA INSIDE THE BEGIN FUNCTION. USE THE MODIFIED LIBRARY

// PINS payload v2  (if not used just comment it out don't delete)
int _SDA = 14;
int _SCL = 15;
TwoWire *_WIRE = &Wire1;
const int PORT_EXTENDER_ADDRESS_I2C = 0x20;
int interval_time = 1000; // ms

const int pin_count = 8;
PinMode pin[pin_count] = {OUTPUT, OUTPUT, OUTPUT, OUTPUT, OUTPUT, OUTPUT, OUTPUT, OUTPUT};

void start()
{
    _WIRE->setSCL(_SCL);
    _WIRE->setSDA(_SDA);
    _WIRE->begin();
    ///!!!! THe LIBRARY WAS MOTIFIED TO NOT CALL SET SCL AND SET SDA and wire BEGIN FUNCTION. USE THE MODIFIED LIBRARY
    PCF8575 *_port_extender = new PCF8575(_WIRE, PORT_EXTENDER_ADDRESS_I2C);
    _port_extender->begin();

    // Set pinMode to OUTPUT
    for (int i = 0; i < 8; i++)
    {
        _port_extender->pinMode(i, pin[i]);
    }

    Serial.println("port extender set");
    int i = 0;
    delay(1000);
    while (true)
    {
        // input doesn't work for some reason
        if (pin[i] == INPUT)
        {
            uint8_t val = _port_extender->digitalRead(i);
            Serial.println("PIN " + String(i) + "  State: " + String(val));
            delay(500);
        }
        else
        {
            _port_extender->digitalWrite(i, HIGH);
            Serial.println("PIN " + String(i) + "  SET TO: HIGH");
            delay(500);
            _port_extender->digitalWrite(i, LOW);
            Serial.println("PIN " + String(i) + "  SET TO: LOW");
        }
        i++;
        {
            if (i > 7)
            {
                i = 0;
            }
        }
    }
}
