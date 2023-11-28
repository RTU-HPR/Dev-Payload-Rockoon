#pragma once
#include <SPI.h>
#include <RadioLib.h>

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

// PINS payload v2  (if not used just comment it out don't delete) ------------------------------------------
struct COM_CONFIG
{
    float FREQUENCY = 434.5;
    int CS = 2;
    int DIO0 = 3;
    int DIO1 = 5;
    int RESET = 8; // schematic was changed from 4 -> 8
    int SYNC_WORD = 0xF4;
    int TXPOWER = 14;
    int SPREADING = 10;
    int CODING_RATE = 7;
    float SIGNAL_BW = 125;
    SPIClassRP2040 *SPI_BUS = &SPI;
};
COM_CONFIG com_config;
int _MOSI = 7;
int _MISO = 4;
int _SCK = 6;
bool transmit = false; // sets the module in transmitting or receiving state
#define radio_module SX1268

void start()
{
    com_config.SPI_BUS->setRX(_MISO);
    com_config.SPI_BUS->setTX(_MOSI);
    com_config.SPI_BUS->setSCK(_SCK);
    com_config.SPI_BUS->begin();

    radio_module radio = new Module(com_config.CS, com_config.DIO0, com_config.RESET, com_config.DIO1, SPI);

    // initialize SX1262 with default settings
    Serial.print(F("[SX1262] Initializing ... "));
    int state = radio.begin();
    if (state == RADIOLIB_ERR_NONE)
    {
        Serial.println(F("success!"));
    }
    else
    {
        Serial.print(F("failed, code "));
        Serial.println(state);
        while (true)
            ;
    }
    radio.setFrequency(com_config.FREQUENCY);
    radio.setSyncWord(com_config.SYNC_WORD);
    radio.setOutputPower(com_config.TXPOWER);
    radio.setSpreadingFactor(com_config.SPREADING);
    radio.setCodingRate(com_config.CODING_RATE);
    radio.setBandwidth(com_config.SIGNAL_BW);
    // setup rf/tx switching
    radio.setDio2AsRfSwitch(true);

    // enable RxBoostedGain mode which will improve the recievers sensitivity at the cost of power consumption
    // test this when everything else works
    // if (int status_rxboost = radio.setRxBoostedGainMode(true, true) == RADIOLIB_ERR_NONE)
    // {
    //     Serial.println("rx gain set successfully");
    // }
    // else
    // {
    //     Serial.println("rx gain set failed: " + String(status_rxboost));
    // }

    // setup tcxo
    if (int status_tcxo = radio.setTCXO(1.8) == RADIOLIB_ERR_NONE)
    {
        Serial.println("TCXO set successfully");
    }
    else
    {
        Serial.println("TCXO set failed: " + String(status_tcxo));
    }

        int count = 0;
    while (true)
    {
        if (transmit)
        {
            Serial.print(F("[SX1262] Transmitting packet ... "));

            // you can transmit C-string or Arduino string up to
            // 256 characters long
            String str = "Hello World! #" + String(count++);
            int state = radio.transmit(str);

            if (state == RADIOLIB_ERR_NONE)
            {
                // the packet was successfully transmitted
                Serial.println(F("success!"));

                // print measured data rate
                Serial.print(F("[SX1262] Datarate:\t"));
                Serial.print(radio.getDataRate());
                Serial.println(F(" bps"));
            }
            else if (state == RADIOLIB_ERR_PACKET_TOO_LONG)
            {
                // the supplied packet was longer than 256 bytes
                Serial.println(F("too long!"));
            }
            else if (state == RADIOLIB_ERR_TX_TIMEOUT)
            {
                // timeout occurred while transmitting packet
                Serial.println(F("timeout!"));
            }
            else
            {
                // some other error occurred
                Serial.print(F("failed, code "));
                Serial.println(state);
            }

            // wait for a second before transmitting again
            delay(1000);
        }
        else
        {
            Serial.print(F("[SX1262] Waiting for incoming transmission ... "));

            // you can receive data as an Arduino String
            String str;
            int state = radio.receive(str);

            if (state == RADIOLIB_ERR_NONE)
            {
                // packet was successfully received
                Serial.println(F("success!"));

                // print the data of the packet
                Serial.print(F("[SX1262] Data:\t\t"));
                Serial.println(str);

                // print the RSSI (Received Signal Strength Indicator)
                // of the last received packet
                Serial.print(F("[SX1262] RSSI:\t\t"));
                Serial.print(radio.getRSSI());
                Serial.println(F(" dBm"));

                // print the SNR (Signal-to-Noise Ratio)
                // of the last received packet
                Serial.print(F("[SX1262] SNR:\t\t"));
                Serial.print(radio.getSNR());
                Serial.println(F(" dB"));

                // print frequency error
                Serial.print(F("[SX1262] Frequency error:\t"));
                Serial.print(radio.getFrequencyError());
                Serial.println(F(" Hz"));
            }
            else if (state == RADIOLIB_ERR_RX_TIMEOUT)
            {
                // timeout occurred while waiting for a packet
                Serial.println(F("timeout!"));
            }
            else if (state == RADIOLIB_ERR_CRC_MISMATCH)
            {
                // packet was received, but is malformed
                Serial.println(F("CRC error!"));
            }
            else
            {
                // some other error occurred
                Serial.print(F("failed, code "));
                Serial.println(state);
            }
        }
    }
}
