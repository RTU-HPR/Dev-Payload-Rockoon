#pragma once
#include "config.h"
#include "core/log.h"
#include "states/ascent_state.h"
#include "states/descent_state.h"
#include "states/prepare_state.h"
#include "sensors/sensor_manager.h"

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Servo.h>
#include <EEPROM.h>

// Cansat object
class Cansat
{
private:
    void init_all_com_bus(Config &config);
    void recover_state_variables(Cansat &cansat);
    void read_last_state(Cansat &cansat);
    void save_to_flash_after_init(Cansat &cansat);

public:
    // Available Cansat states
    enum State
    {
        PREP,
        ASCENT,
        DESCENT,
    };
    
    bool _has_recovered_to_state = false;

    State current_state = State::PREP;
    Config config;
    Log log;
    Sensor_manager sensors;

    String receive_command(Cansat &cansat);
    void restart(Cansat &cansat);
    void check_if_should_restart(Cansat &cansat);
    void start_states(Cansat &cansat);
    void save_last_state(Cansat &cansat);
};
