#include <Arduino.h>
#include <SPI.h>
#include "ranging_wrapper.h"
#include "RFM_wrapper.h"
bool transmiting_mode = false; // dont change

String ARM_MSG = "arm_confirm";
String DATA_MSG = "data_send";
String DATA_STOP_MSG = "data_stop";
String HEATER_ENABLE = "heater_enable";

RFM_Wrapper com_lora;
RFM_Wrapper::Lora_Device com_config = {.FREQUENCY = 434.5,
                                       .CS = 7,
                                       .DIO0 = 5,
                                       .DIO1 = 4,
                                       .RESET = 6,
                                       .SYNC_WORD = 0xF4,
                                       .TXPOWER = 14,
                                       .SPREADING = 10,
                                       .CODING_RATE = 7,
                                       .SIGNAL_BW = 125,
                                       .SPI = &SPI1};

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

void read_main_lora()
{
    if (!com_lora.get_init_status())
    {
        return;
    }
    String msg;
    float rssi = 0, snr = 0;
    if (!com_lora.recieve(msg, rssi, snr))
    {
        return;
    }

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
    if (!com_lora.get_init_status())
    {
        return;
    }
    while (com_lora.send(msg) == false)
    {
        delay(50);
    }

    return;
}

void init_LoRa_main(RFM_Wrapper::Lora_Device lora_cfg)
{
    // delete &lora;
    String status = com_lora.init(true, com_config);
    if (com_lora.get_init_status())
    {
        Serial.println("RFM lora good");
    }
    else
    {
        Serial.println(status);
    }
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
        init_LoRa_main(com_config);
    }
    int_LoRa_ranging(ranging_device);
    Serial.println("base station setup done");
}

void loop()
{
    if (transmiting_mode)
    {
        if (Serial.available() > 0)
        {
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
        read_main_lora();
        if (Serial.available() > 0)
        {
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
                    Serial.println("Unexpected input : " + incoming_msg);
                }
            }
        }
    }

    /// enable raning slave
    if (ranging_lora.get_init_status())
    {
        bool result = ranging_lora.slave_reenable(10000, RANGING_SLAVE);
        if (result == true)
        {
            Serial.println("ranging ping recieved");
            pinMode(buzzer_pin, OUTPUT_12MA);
            // digitalWrite(buzzer_pin, HIGH);
            tone(buzzer_pin, 1000, 250);
        }
    }
}
