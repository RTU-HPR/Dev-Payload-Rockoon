#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

/*
1. Simply include the wanted test file using #include   !! dont include more than one
2. make sure the included file has the right pins
3. run the start function
*/

#include <ms5611_test.h>

void setup()
{

    Serial.begin(115200); // initialize serial

    // Add start here
    start();

    Serial.println("------------------- setup done ------------------");
}

void loop()
{
}
