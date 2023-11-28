#pragma once
#include "RadioLib_wrapper.h"
#include <SPI.h>

// PINS payload FC v1  (if not used just comment it out don't delete) ------------------------------------------
// struct COM_CONFIG
// {
//     float FREQUENCY = 434.5;
//     int CS = 5;
//     int DIO0 = 7;
//     int DIO1 = 8;
//     int RESET = 6; // schematic was changed from 4 -> 8
//     int SYNC_WORD = 0xF4;
//     int TXPOWER = 14;
//     int SPREADING = 10;
//     int CODING_RATE = 7;
//     float SIGNAL_BW = 125;
//     SPIClassRP2040 *SPI_BUS = &SPI;
// };
// COM_CONFIG com_config;
// int _MOSI = 3;
// int _MISO = 4;
// int _SCK = 2;
// bool transmit = true; // sets the module in transmitting or receiving state
// #define radio_module RFM96

// PINS payload BASE STATION v1  (if not used just comment it out don't delete) ------------------------------------------

// struct COM_CONFIG
// {
//     float FREQUENCY = 434.5;
//     int CS = 7;
//     int DIO0 = 5;
//     int DIO1 = 4;
//     int RESET = 6; // schematic was changed from 4 -> 8
//     int SYNC_WORD = 0xF4;
//     int TXPOWER = 14;
//     int SPREADING = 10;
//     int CODING_RATE = 7;
//     float SIGNAL_BW = 125;
//     SPIClassRP2040 *SPI_BUS = &SPI1;
// };
// COM_CONFIG com_config;
// int _MOSI = 11;
// int _MISO = 12;
// int _SCK = 10;
// bool transmit = false; // sets the module in transmitting or receiving state
// #define radio_module RFM96

// // PINS payload v2  (if not used just comment it out don't delete) ------------------------------------------
// struct COM_CONFIG
// {
//     float FREQUENCY = 434.5;
//     int CS = 2;
//     int DIO0 = 3;
//     int DIO1 = 5;
//     int RESET = 8; // schematic was changed from 4 -> 8
//     int SYNC_WORD = 0xF4;
//     int TXPOWER = 14;
//     int SPREADING = 10;
//     int CODING_RATE = 7;
//     float SIGNAL_BW = 125;
//     SPIClassRP2040 *SPI_BUS = &SPI;
// };

int _MOSI = 7;
int _MISO = 4;
int _SCK = 6;
bool transmit = false; // sets the module in transmitting or receiving state
#define radio_module SX1268
RadioLib_Wrapper<radio_module>::RADIO_CONFIG com_config{
    .FREQUENCY = 434.5,
    .CS = = 2,
    .DIO0 = 3,
    .DIO1 = 5,
    .FAMILY = RadioLib_Wrapper<radio_module>::RADIO_CONFIG::CHIP_FAMILY::SX126X,
    .rf_switching = RadioLib_Wrapper<radio_module>::RADIO_CONFIG::RF_SWITCHING::DIO2,
    // .RX_ENABLE = 0, // only needed if rf_switching = gpio
    // .TX_ENABLE = 0, // only needed if rf_switching = gpio
    .RESET = 8,
    .SYNC_WORD = 0xF4,
    .TXPOWER = 14,
    .SPREADING = 10,
    .CODING_RATE = 7,
    .SIGNAL_BW = 125,
    .SPI_BUS = &SPI,
};

void start()
{
    SPIClassRP2040 *SPI_BUS = (SPIClassRP2040 *)com_config.SPI_BUS;
    SPI_BUS->setRX(_MISO);
    SPI_BUS->setTX(_MOSI);
    SPI_BUS->setSCK(_SCK);
    SPI_BUS->begin();

    RadioLib_Wrapper<radio_module> *_com_lora = new RadioLib_Wrapper<radio_module>(com_config);

    bool configure_status = _com_lora->configure_radio(com_config);
    if (!configure_status)
    {
        Serial.println("Configuration failed)");
    }
    else
    {
        Serial.println("Module init done");
    }
    // _com_lora->test_transmit();

    while (true)
    {
        if (transmit)
        {
            // transmit
            String msg = "Hello World";
            _com_lora->add_checksum(msg);
            bool sent_status = _com_lora->transmit(msg);
            if (sent_status)
            {
                Serial.println("MSG sent");
            }
            else
            {
                // Serial.println("Msg not sent");
            }
            delay(1000);
        }
        else
        {
            // receive
            String msg;
            float rssi = 0;
            float snr = 0;
            bool received_status = _com_lora->receive(msg, rssi, snr);
            if (received_status)
            {
                Serial.println("Message received RAW MSG: " + msg + "  RSSI: " + String(rssi) + "   SNR: " + String(snr));
                if (_com_lora->check_checksum(msg))
                {
                    Serial.println("Check sum good: " + msg);
                }
                else
                {
                    Serial.println("Check sum BAD: " + msg);
                }
            }
        }
    }
}
