#pragma once
#include <SPI.h>
#include <Wire.h>

// PINS   (if not used just comment it out don't delete)

void start()
{
    int _SDA = 14;
    int _SCL = 15;

    Wire1.setSCL(15);
    Wire1.setSDA(14);
    Wire1.begin();
    while (true)
    {
      byte error, address;
      int nDevices;
    
      Serial.println("Scanning...");
    
      nDevices = 0;
      for(address = 1; address < 127; address++ )
      {
        // The i2c_scanner uses the return value of
        // the Write.endTransmisstion to see if
        // a device did acknowledge to the address.
        Wire1.beginTransmission(address);
        error = Wire1.endTransmission();
    
        if (error == 0)
        {
          Serial.print("I2C device found at address 0x");
          if (address<16)
            Serial.print("0");
          Serial.print(address,HEX);
          Serial.println("  !");
    
          nDevices++;
        }
        else if (error==4)
        {
          Serial.print("Unknown error at address 0x");
          if (address<16)
            Serial.print("0");
          Serial.println(address,HEX);
        }    
      }
      if (nDevices == 0)
        Serial.println("No I2C devices found\n");
      else
        Serial.println("done\n");
    
      delay(5000);           // wait 5 seconds for next scan
    }
}
