#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include "ranging_wrapper.h"
#include <RadioLib_wrapper.h>

String ARM_MSG = "arm_confirm";
String DATA_MSG = "data_send";
String DATA_STOP_MSG = "data_stop";
String HEATER_ENABLE = "heater_enable";

// 433 MHz LoRa
#define radio_module RFM96
RadioLib_Wrapper<radio_module> *com_lora;
RadioLib_Wrapper<radio_module>::RADIO_CONFIG com_config{
    .FREQUENCY = 434.5,
    .CS = 7,
    .DIO0 = 5,
    .DIO1 = 4,
    .FAMILY = RadioLib_Wrapper<radio_module>::RADIO_CONFIG::CHIP_FAMILY::SX127X,
    .rf_switching = RadioLib_Wrapper<radio_module>::RADIO_CONFIG::RF_SWITCHING::DISABLED,
    // .RX_ENABLE = 0, // only needed if rf_switching = gpio
    // .TX_ENABLE = 0, // only needed if rf_switching = gpio
    .RESET = 6,
    .SYNC_WORD = 0xF4,
    .TXPOWER = 14,
    .SPREADING = 10,
    .CODING_RATE = 7,
    .SIGNAL_BW = 125,
    .SPI_BUS = &SPI1,
};

/*
struct COM_CONFIG
{
    float FREQUENCY = 434.5;
    int CS = 7;
    int DIO0 = 5;
    int DIO1 = 4;
    int RESET = 6;
    int SYNC_WORD = 0xF4;
    int TXPOWER = 14;
    int SPREADING = 10;
    int CODING_RATE = 7;
    float SIGNAL_BW = 125;
    HardwareSPI *SPI_BUS = &SPI1;
};

COM_CONFIG com_config;
*/

Ranging_Wrapper::Mode LORA2400_MODE = Ranging_Wrapper::Mode::SLAVE;
Ranging_Wrapper ranging_lora;
Ranging_Wrapper::Ranging_Slave RANGING_SLAVE = {.position = {0, 0, 0}, .address = 0x9A8B7C6D}; // posible addreses 0x12345678 , 0xABCD9876 , 0x9A8B7C6D
Ranging_Wrapper::Lora_Device ranging_device = {.FREQUENCY = 2405.6,
                                               .CS = 17,
                                               .DIO0 = 22, // busy
                                               .DIO1 = 21,
                                               .RESET = 20,
                                               .SYNC_WORD = 0xF5,
                                               .TXPOWER = 14,
                                               .SPREADING = 10,
                                               .CODING_RATE = 7,
                                               .SIGNAL_BW = 406.25,
                                               .SPI = &SPI};
const int buzzer_pin = 3;
const int buzz_length = 200;
bool master_basestation = true;
bool transmiting_mode = false;

void read_main_lora()
{
    String msg = "";
    float rssi;
    float snr;

    // Get data from LoRa
    if (!com_lora->receive(msg, rssi, snr))
    {
        return;
    }

    bool status = com_lora->check_checksum(msg);
    if (!status)
    {
        String error_msg = "Message checksum fail";
        // Serial.println(error_msg);
    }
    // Serial.println("Message received: " + msg);

    if (msg.charAt(0) == '!')
    {
        Serial.print(msg);
        Serial.println(", " + String(rssi, 2) + ", " + String(snr, 2));
    }
    else
    {
        // format for visualiser
        Serial.print("/*");
        Serial.print(msg);
        Serial.print(", " + String(rssi, 2) + ", " + String(snr, 2));
        Serial.println("*/");
    }
}

void send_main_lora(String msg)
{
    com_lora->add_checksum(msg);
    com_lora->transmit(msg);
}

void init_LoRa_main()
{
    // Init LoRa
    com_lora = new RadioLib_Wrapper<radio_module>(com_config);
    bool status = com_lora->configure_radio(com_config);
    if (!status)
    {
        String msg = "Configuring LoRa failed";
        Serial.println(msg);
    }
    com_lora->test_transmit();
}

void int_LoRa_ranging(Ranging_Wrapper::Lora_Device lora_cfg)
{
    String status = ranging_lora.init(LORA2400_MODE, lora_cfg);
    if (ranging_lora.get_init_status())
    {
        Serial.println("Ranging lora good");
    }
    else
    {
        Serial.println(status);
    }
}

void setup()
{

    Serial.begin(115200); // initialize serial
    if (master_basestation)
    {
        while (!Serial)
        {
            delay(100);
        }
    }
    SPI1.setSCK(10);
    SPI1.setRX(12);
    SPI1.setTX(11);
    SPI1.begin();
    SPI.setSCK(18);
    SPI.setRX(16);
    SPI.setTX(19);
    SPI.begin();
    if (master_basestation)
    {
        init_LoRa_main();
    }
    int_LoRa_ranging(ranging_device);
    Serial.println("base station setup done");
}

void loop()
{
    if (transmiting_mode)
    {
        Serial.println("Transmit");
        if (Serial.available() > 0)
        {   
            Serial.println("Listening to serial transmit");
            String incoming_msg = Serial.readString();
            if (incoming_msg != "")
            {
                incoming_msg.trim();
                if (incoming_msg == "D")
                {
                    send_main_lora(DATA_MSG);
                    Serial.println("Switching to recieving mode");
                    transmiting_mode = false;
                }
                else if (incoming_msg == "S")
                {
                    send_main_lora(DATA_STOP_MSG);
                    Serial.println("Switching to recieving mode");
                    transmiting_mode = false;
                }
                else if (incoming_msg == "H")
                {
                    send_main_lora(HEATER_ENABLE);
                    Serial.println("Switching to recieving mode");
                    transmiting_mode = false;
                }
                else if (incoming_msg == "A")
                {
                    send_main_lora(ARM_MSG);
                    Serial.println("Switching to recieving mode");
                    transmiting_mode = false;
                }
                else if (incoming_msg == "R")
                {
                    Serial.println("Switching to recieving mode");
                    transmiting_mode = false;
                }
                else
                {
                    Serial.println("Unexpected input : " + incoming_msg);
                }
            }
        }
    }
    else
    {
        String msg = "";
        float rssi;
        float snr;

        // Get data from LoRa
        if (com_lora->receive(msg, rssi, snr))
        {
            bool status = com_lora->check_checksum(msg);
            if (!status)
            {
                String error_msg = "Message checksum fail";
                // Serial.println(error_msg);
            }
            Serial.println("Message received: " + msg);
        }

        if (Serial.available() > 0)
        {
            Serial.println("Listening to serial transmit");
            String incoming_msg = Serial.readString();
            if (incoming_msg != "")
            {
                incoming_msg.trim();
                if (incoming_msg == "T")
                {
                    Serial.println("Switching to transmiting mode");
                    transmiting_mode = true;
                }
                else
                {
                    transmiting_mode = true;
                    Serial.println("Unexpected input : " + incoming_msg);
                }
            }
        }
    }
}
