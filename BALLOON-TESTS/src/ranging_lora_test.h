#pragma once
#include <SPI.h>
#include <Wire.h>
#include <ranging_wrapper.h>

// PINS payload v2  (if not used just comment it out don't delete) ------------------------------------------
Ranging_Wrapper::Ranging_Slave RANGING_SLAVES[3] = {{.position = {0, 0, 0}, .address = 0x12345678},
                                                    {.position = {0, 0, 0}, .address = 0xABCD9876},
                                                    {.position = {0, 0, 0}, .address = 0x9A8B7C6D}};

Ranging_Wrapper::Mode LORA2400_MODE = Ranging_Wrapper::Mode::MASTER;
Ranging_Wrapper::Lora_Device ranging_device = {.FREQUENCY = 2405.6,
                                               .CS = 10,
                                               .DIO0 = 13, // busy
                                               .DIO1 = 12,
                                               .RESET = 11,
                                               .SYNC_WORD = 0xF5,
                                               .TXPOWER = 14,
                                               .SPREADING = 10,
                                               .CODING_RATE = 7,
                                               .SIGNAL_BW = 406.25,
                                               .SPI = &SPI};
const int SPI0_RX = 4; // schematic was changed from 8 -> 4
const int SPI0_TX = 7;
const int SPI0_SCK = 6;

void start()
{
    SPI.setRX(4);
    SPI.setTX(7);
    SPI.setSCK(6);
    SPI.begin();

    Ranging_Wrapper _ranging_lora;
    unsigned long _last_ranging_pos_time = 0;
    bool _ranging_lora_initalized = false;
    int _last_slave_index = 0;
    int _slave_index = 0;
    String status = "";

    String result = _ranging_lora.init(LORA2400_MODE, ranging_device);
    if (result == "")
    {
        _ranging_lora_initalized = true;
        Serial.println("All good");
    }
    else
    {
        status += result;
        Serial.println("Failed: " + status);
    }

    while (true)
    {

        if (_ranging_lora_initalized)
        {
            Ranging_Wrapper::Ranging_Result result = {0, 0};
            bool move_to_next_slave = false;
            unsigned long timeout = 500;
            if (_ranging_lora.master_read(RANGING_SLAVES[_slave_index], result, timeout))
            {
                // ranging data read and ranging for current slave started
                move_to_next_slave = true;
            }
            // check if something useful was read from the previous slave
            if (result.distance != 0 && result.time != 0)
            {
                Serial.println(String(_last_slave_index) + " Distance:  " + String(result.distance, 2) + "  Time:" + String(result.time));
                Serial.println(String(_last_slave_index) + " RSSI:  " + String(result.rssi, 2) + "  SNR:" + String(result.snr));
            }

            // move to next slave
            if (move_to_next_slave)
            {
                _last_slave_index = _slave_index;
                int array_length = 3;
                _slave_index++;
                if (_slave_index > array_length - 1)
                {
                    // reset index
                    _slave_index = 0;
                }
            }
        }
    }
}
