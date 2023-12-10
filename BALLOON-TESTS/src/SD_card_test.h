#pragma once
#include <SPI.h>
#include <Wire.h>
#include <SDFS.h>
// PINS   (if not used just comment it out don't delete)
int _MOSI = 7;
int _MISO = 4;
int _SCK = 6;
int _CS = 9;
SPIClassRP2040 *SPI_BUS = &SPI;

void start()
{
    SPI_BUS->setRX(_MISO);
    SPI_BUS->setTX(_MOSI);
    SPI_BUS->setSCK(_SCK);
    SPI_BUS->begin();

    // Config
    FS *_flash = &SDFS;
    SDFSConfig sd_config;
    sd_config.setCSPin(_CS);
    sd_config.setSPI(*SPI_BUS);

    if (_flash->setConfig(sd_config))
    {
        Serial.println("Config set");
    }
    else
    {
        Serial.println("Config not set");
    }

    // Initialize flash
    if (_flash->begin())
    {
        Serial.println("FileSystem init success");
    }
    else
    {
        Serial.println("FileSystem init error");
        return;
    }

    int i = 0;
    while (true)
    {
        i++;
        // write to flash
        File file = _flash->open("sc_card_test.txt", "a+");
        if (!file)
        {
            Serial.println("Failed opening file: " + String("sc_card_test.txt"));
            return;
        }
        file.println(String(i));
        file.close();

        Serial.println("Wrote " + String(i) + " to SD card");
        delay(1000);
    }
}
