#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_Sensor.h>
#include <EEPROM.h>
#include <PCF8575.h>


/*
1. Simply include the wanted test file using #include   !! dont include more than one
2. make sure the included file has the right pins
3. run the start function
*/

#include <lora_test.h>

void setup()
{
    int _SDA = 14;
    int _SCL = 15;
    TwoWire *_WIRE = &Wire1;
    const int PORT_EXTENDER_ADDRESS_I2C = 0x20;
    int interval_time = 1000; // ms

    const int pin_count = 8;
    PinMode pin[pin_count] = {OUTPUT, OUTPUT, OUTPUT, OUTPUT, OUTPUT, OUTPUT, OUTPUT, OUTPUT};

    _WIRE->setSCL(_SCL);
    _WIRE->setSDA(_SDA);
    _WIRE->begin();
    ///!!!! THe LIBRARY WAS MOTIFIED TO NOT CALL SET SCL AND SET SDA INSIDE THE BEGIN FUNCTION. USE THE MODIFIED LIBRARY
    PCF8575 *_port_extender = new PCF8575(_WIRE, PORT_EXTENDER_ADDRESS_I2C);
    _port_extender->begin();

    Serial.println("port extender set");
    delay(1000);
    _port_extender->pinMode(0, OUTPUT);
    _port_extender->pinMode(1, OUTPUT);
    _port_extender->pinMode(2, OUTPUT);
    _port_extender->pinMode(3, OUTPUT);
    _port_extender->pinMode(4, OUTPUT);
    _port_extender->pinMode(5, OUTPUT);
    _port_extender->pinMode(6, OUTPUT);
    _port_extender->pinMode(7, OUTPUT);

    _port_extender->digitalWrite(0, LOW);
    _port_extender->digitalWrite(1, LOW);
    _port_extender->digitalWrite(3, LOW);
    _port_extender->digitalWrite(4, LOW);
    _port_extender->digitalWrite(5, LOW);
    _port_extender->digitalWrite(6, LOW);
    _port_extender->digitalWrite(7, LOW);

    Serial.begin(115200); // initialize serial
    while (!Serial)
    {
        delay(100);
    }
    delay(1000);
    Serial.println("------------------- starting ------------------");
    // Add start here
    start();

    Serial.println("------------------- setup done ------------------");
}

void loop()
{
}
