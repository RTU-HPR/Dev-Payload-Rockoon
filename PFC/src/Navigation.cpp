#include <Navigation.h>

bool Navigation::beginGps(const Gps_Wrapper::Gps_Config_I2C &gps_config)
{
    _gps = Gps_Wrapper(nullptr, "GPS");
    unsigned long start = millis();    
    while (!_gps.begin(gps_config))
    {
        if (millis() - start > 10000)
        {
            Serial.println("Auto GPS begin failed");
            Serial.println("Reseting GPS module");
            // Reset the gps module
            pinMode(22, OUTPUT_12MA);
            digitalWrite(22, LOW);
            delay(1000);
            digitalWrite(22, HIGH);
            delay(1000);
            digitalWrite(22, LOW);
            if (!_gps.begin(gps_config))
            {
                Serial.println("GPS begin failed after reset");
                return false;
            }
            return true;
        }
        // Serial.println("GPS begin failed, retrying");
    }

    return true;
}

bool Navigation::beginRanging(const Ranging_Wrapper::Lora_Device &ranging_config, const Ranging_Wrapper::Mode &ranging_mode)
{
    String result = _ranging.init(ranging_mode, ranging_config);
    if (result == "")
    {
        ranging_initalized = true;
    }
    else
    {
        Serial.println("Ranging initialization failed with error: " + result);
        return false;
    }
    // Default values
    navigation_data.ranging_position = Ranging_Wrapper::Position(0, 0, 0);
    
    return true;
}

bool Navigation::readGps(NAVIGATION_DATA &navigation_data)
{
    bool position_valid = false;
    bool time_valid = false;
    if (!_gps.read(navigation_data.gps, position_valid, time_valid))
    {
      return false;
    }
    return true;
}

bool Navigation::readRanging(Config &config, NAVIGATION_DATA &navigation_data)
{
    if (!ranging_initalized)
    {
        return false;
    }

    Ranging_Wrapper::Ranging_Result result = {0, 0};
    bool move_to_next_slave = false;
    if (_ranging.master_read(config.RANGING_SLAVES[_slave_index], result, config.RANGING_LORA_TIMEOUT))
    {
        // ranging data read and ranging for current slave started
        move_to_next_slave = true;
    }
    // check if something useful was read from the previous slave
    if (result.distance != 0 && result.time != 0)
    {
        navigation_data.ranging[_last_slave_index] = result;
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
    // If all slaves have been read, calculate position
    else
    {
        Ranging_Wrapper::Position result;
        if (_ranging.trilaterate_position(navigation_data.ranging, config.RANGING_SLAVES, result))
        {
            navigation_data.ranging_position = result;
        }
    }
    return true;
}