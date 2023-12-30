#include <Arduino.h>
#include <SPI.h>
#include <EEPROM.h>
#include <Sd_card_wrapper.h>

SD_Card_Wrapper sd_card_wrapper(nullptr, "SD");
// spi pins
const int SPI0_RX = 4; // schematic was changed from 8 -> 4
const int SPI0_TX = 7;
const int SPI0_SCK = 6;
const int SD_CARD_CS = 9;
// path: Sd_card_wrapper.h
SD_Card_Wrapper::Config sd_card_config = {
    .spi_bus = &SPI,
    .cs_pin = SD_CARD_CS,
    .data_file_path_base = "/data",
    .info_file_path_base = "/info",
    .error_file_path_base = "/error",
    .config_file_path = "/config",
    .data_file_header = "index, lat, lon",
    .info_file_header = "index, time, info",
    .error_file_header = "index, time, error",
    .config_file_header = "rx, tx, sck, cs",
    .open_last_files = true,
};

void init_sd()
{
    if (!sd_card_wrapper.init(sd_card_config))
    {
        Serial.println("SD card init failed");
        while (true)
            ;
    }
    if (!sd_card_wrapper.clean_storage(sd_card_config))
    {
        Serial.println("SD card clean failed");
        while (true)
            ;
    }
    Serial.println("SD card init good");
}
void initial_read()
{
    String data;
    if (sd_card_wrapper.read_data(data))
    {
        Serial.println("data read: " + data);
    }
    String info;
    if (sd_card_wrapper.read_info(info))
    {
        Serial.println("info read: " + info);
    }

    String error;
    if (sd_card_wrapper.read_error(error))
    {
        Serial.println("error read: " + error);
    }

    String config;
    if (sd_card_wrapper.read_config(config))
    {
        Serial.println("config read: " + config);
    }
}

void write_test()
{
    if (!sd_card_wrapper.write_data("1, 77.77, 77.777"))
    {
        Serial.println("SD card write data failed");
        while (true)
            ;
    }
    if (!sd_card_wrapper.write_data("420, 77.777, 77.7777"))
    {
        Serial.println("SD card write data failed");
        while (true)
            ;
    }
    if (!sd_card_wrapper.write_info("1, " + String(millis()) + ", example info"))
    {
        Serial.println("SD card write info failed");
        while (true)
            ;
    }
    if (!sd_card_wrapper.write_error("1, " + String(millis()) + ", example error"))
    {
        Serial.println("SD card write error failed");
        while (true)
            ;
    }
    if (!sd_card_wrapper.write_config("1, 2, 3, 4", sd_card_config))
    {
        Serial.println("SD card write config failed");
        while (true)
            ;
    }
}

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        delay(500);
    }
    SPI.setRX(SPI0_RX);
    SPI.setTX(SPI0_TX);
    SPI.setSCK(SPI0_SCK);
    SPI.begin();

    init_sd();
    initial_read();
    write_test();
    Serial.println("SD card write done: reading again");
    delay(1000);
    initial_read();

    Serial.println("SD card write test done: unplug the device in 30sec otherwise it will format itself");
    delay(30000);

    if (!sd_card_wrapper.clean_storage(sd_card_config))
    {
        Serial.println("SD card clean failed");
        while (true)
            ;
    }
}

void loop()
{
    Serial.println("loop");
    delay(1000);
    // Your main loop code here
}
