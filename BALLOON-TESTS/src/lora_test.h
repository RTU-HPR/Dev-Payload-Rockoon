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

struct COM_CONFIG
{
    float FREQUENCY = 434.5;
    int CS = 7;
    int DIO0 = 5;
    int DIO1 = 4;
    int RESET = 6; // schematic was changed from 4 -> 8
    int SYNC_WORD = 0xF4;
    int TXPOWER = 14;
    int SPREADING = 10;
    int CODING_RATE = 7;
    float SIGNAL_BW = 125;
    SPIClassRP2040 *SPI_BUS = &SPI1;
};
COM_CONFIG com_config;
int _MOSI = 11;
int _MISO = 12;
int _SCK = 10;
bool transmit = false; // sets the module in transmitting or receiving state
#define radio_module RFM96

// PINS payload v2  (if not used just comment it out don't delete) ------------------------------------------
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
// COM_CONFIG com_config;
// int _MOSI = 7;
// int _MISO = 4;
// int _SCK = 6;
// bool transmit = true; // sets the module in transmitting or receiving state
// #define radio_module SX1268

void start()
{
    com_config.SPI_BUS->setRX(_MISO);
    com_config.SPI_BUS->setTX(_MOSI);
    com_config.SPI_BUS->setSCK(_SCK);
    com_config.SPI_BUS->begin();
    RadioLib_Wrapper<radio_module> *_com_lora = new RadioLib_Wrapper<radio_module>(com_config.CS, com_config.DIO0, com_config.RESET, com_config.DIO1, com_config.SPI_BUS);
    bool configure_status = _com_lora->configure_radio(com_config.FREQUENCY, com_config.TXPOWER, com_config.SPREADING, com_config.CODING_RATE, com_config.SIGNAL_BW, com_config.SYNC_WORD);
    if (!configure_status)
    {
        Serial.println("Configuration failed)");
    }
    else
    {
        Serial.println("Module init done");
    }
    _com_lora->test_transmit();

    while (true)
    {
        if (transmit)
        {
            // transmit
            bool sent_status = _com_lora->transmit("Hello World");
            if (sent_status)
            {
                Serial.println("MSG sent");
            }
            else
            {
                // Serial.println("Msg not sent");
                // delay(1000);
            }
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
                Serial.println("Message received: " + msg + "  RSSI: " + String(rssi) + "   SNR: " + String(snr));
            }
        }
    }
}
