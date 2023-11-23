#pragma once
#include <RadioLib.h>
#include <Arduino.h>

class RFM_Wrapper
{
public:
    struct Lora_Device
    {
        float FREQUENCY;
        int CS;
        int DIO0; // busy pin
        int DIO1;
        int RESET;
        int SYNC_WORD;

        int TXPOWER;
        int SPREADING;
        int CODING_RATE;
        float SIGNAL_BW;
        SPIClassRP2040 *SPI;
    };

private:
    RFM96 _lora = new Module(-1, -1, -1);
    bool _lora_initialized = false;

    int _action_state = RADIOLIB_ERR_NONE;
    bool _last_action_transmit; // indicates which was the last action

    Lora_Device _config;
    String _recieve_left_over = ""; // left overs which will be given back if starting recieving call

public:
    String init(bool transmit_first, Lora_Device config);
    bool send(String msg);
    bool recieve(String &msg, float &rssi, float &snr);
    bool get_init_status();
};