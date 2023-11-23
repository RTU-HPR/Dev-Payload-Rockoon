#include "RFM_wrapper.h"

volatile bool action_done = false;
void set_flag(void)
{
    // we sent or received  packet, set the flag
    action_done = true;
}

bool RFM_Wrapper::get_init_status()
{
    return _lora_initialized;
}

String RFM_Wrapper::init(bool transmit_first, Lora_Device config)
{
    _lora_initialized = false;
    _lora = new Module(config.CS, config.DIO0, config.RESET, config.DIO1, *config.SPI);
    int state = _lora.begin();
    if (state != RADIOLIB_ERR_NONE)
    {
        _lora_initialized = false;
        return "RFM lora failed state: " + String(state);
    }
    // setting paramaters
    _lora.setFrequency(config.FREQUENCY);
    _lora.setOutputPower(config.TXPOWER);
    _lora.setSpreadingFactor(config.SPREADING);
    _lora.setCodingRate(config.CODING_RATE);
    _lora.setBandwidth(config.SIGNAL_BW);
    _lora.setSyncWord(config.SYNC_WORD);

    _lora.setDio0Action(set_flag, RISING);

    if (transmit_first)
    {
        int _action_state = _lora.startTransmit("LoRa Init");
        if (_action_state != RADIOLIB_ERR_NONE)
        {
            _lora_initialized = false;
            return "Transmit starting failed:" + String(_action_state);
        }
        else
        {
            _lora_initialized = true;
            _last_action_transmit = true;
            Serial.println("Transmit started");
            while (!action_done)
            {
                delay(50);
            }

            if (_action_state == RADIOLIB_ERR_NONE)
            {
                return "Transmit Good";
            }
            else
            {
                _lora_initialized = false;
                return "Transmit Bad : " + String(_action_state);
            }
        }
    }
    else
    {
        int recievie_state = _lora.startReceive();
        if (recievie_state != RADIOLIB_ERR_NONE)
        {
            _lora_initialized = false;
            return "Recieve starting failed:" + String(recievie_state);
        }
        else
        {
            _lora_initialized = true;
            _last_action_transmit = false;
            return "Recieve started";
        }
    }
}
// if sent will return true
bool RFM_Wrapper::send(String msg)
{
    if (!action_done && !_last_action_transmit)
    {
        // for some reason if it doesn't timeout its sad mybe later ad timout
        _lora.standby();
        action_done = true;
    }

    if (action_done == false)
    {
        return false;
    }
    else
    {
        // reset flag
        action_done = false;
    }
    if (!_last_action_transmit)
    {
        String str;
        int _action_state = _lora.readData(str);
        if (_action_state == RADIOLIB_ERR_NONE)
        {
            _recieve_left_over = str;
        }
        _action_state == RADIOLIB_ERR_NONE;
    }
    else
    {
        Serial.println("Transmit finished");
        _action_state == RADIOLIB_ERR_NONE;
    }
    // start tranmist
    _lora.finishTransmit();
    _action_state = _lora.startTransmit(msg);
    _last_action_transmit = true;
    if (_action_state != RADIOLIB_ERR_NONE)
    {
        return false;
    }
    else
    {
        return true;
    }
}

// if recieved will return true
bool RFM_Wrapper::recieve(String &msg, float &rssi, float &snr)
{
    // send back anything that was left over when recieve was called and then someone called transmit
    if (_recieve_left_over != "")
    {
        msg = _recieve_left_over;
        _recieve_left_over = "";
        return true;
    }

    if (action_done == false)
    {
        return false;
    }
    else
    {
        // reset flag
        action_done = false;
    }

    if (_last_action_transmit)
    {
        Serial.println("Transmit finished");
        _action_state == RADIOLIB_ERR_NONE;
    }
    else
    {
        // get recieve data
        String str;
        int _action_state = _lora.readData(str);
        if (_action_state == RADIOLIB_ERR_NONE)
        {
            msg = str; /// mybe later add rssi
            rssi = _lora.getRSSI();
            snr = _lora.getSNR();
        }
        // reset state
        _action_state = RADIOLIB_ERR_NONE;
    }
    // start recieve
    _lora.finishTransmit();
    _lora.startReceive();
    _last_action_transmit = false;

    if (msg != "")
    {
        return true;
    }
    else
    {
        return false;
    }
}