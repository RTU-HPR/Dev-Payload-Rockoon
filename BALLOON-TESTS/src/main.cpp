#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_Sensor.h>
#include <EEPROM.h>

/*
1. Simply include the wanted test file using #include   !! dont include more than one
2. make sure the included file has the right pins
3. run the start function
*/

#include <lora_lowlevel_test.h>

void setup()
{

    Serial.begin(115200); // initialize serial
    while (!Serial)
    {
        delay(100);
    }
    delay(3000);
    Serial.println("------------------- starting ------------------");
    // Add start here
    start();

    Serial.println("------------------- setup done ------------------");
}

void loop()
{
}
