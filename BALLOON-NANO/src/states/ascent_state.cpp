#include "states/ascent_state.h"

unsigned long int last_data_transmit_time_ascent = 0;
unsigned long int last_state_save_time_ascent = 0;
unsigned int launch_rail_removed_cycle_count = 0;

bool data_request = false;
bool ranging_request = false;
bool toggle_mosfet_1 = false;
bool toggle_mosfet_2 = false;

unsigned long int ranging_time_start = 0;
unsigned long int last_ranging_time = 0;

unsigned long int toggle_mosfet_1_time = 0;
unsigned long int toggle_mosfet_2_time = 0;

// HELPER FUNCTIONS
void process_requests_ascent(Cansat &cansat)
{
    // Check if data should be sent over LoRa
    // Can only send if minute's seconds' are divisible by 10
    if (data_request == true && (cansat.sensors.data.gps_epoch_time % 100) % 10 == 0)
    {
        // Send data by LoRa
        cansat.log.send_data(cansat.sensors.sendable_packet, cansat.sensors.loggable_packet, true, false, false);
        //data_request = false;
    }

    if (ranging_request == true && ((cansat.sensors.data.gps_epoch_time % 100) % 5 == 0))
    {
        ranging_time_start = millis();
        last_ranging_time = millis();
        cansat.sensors.read_ranging(cansat.config);
        data_request = true;
        //ranging_request = false;
    }

    if (toggle_mosfet_1 == true)
    {
        digitalWrite(cansat.config.PARACHUTE_MOSFET_1, HIGH);
        toggle_mosfet_1_time = millis();
        toggle_mosfet_1 = false;
    }

    if (toggle_mosfet_2 == true)
    {
        digitalWrite(cansat.config.PARACHUTE_MOSFET_2, HIGH);
        toggle_mosfet_2_time = millis();
        toggle_mosfet_2 = false;
    }

    if (millis() - ranging_time_start < 5000)
    {
        if (millis() - last_ranging_time > 500)
        {
            cansat.sensors.read_ranging(cansat.config);
            last_ranging_time = millis();
        }
    }

    if (millis() - toggle_mosfet_1_time > 1000)
    {
        digitalWrite(cansat.config.PARACHUTE_MOSFET_1, LOW);
    }

    if (millis() - toggle_mosfet_2_time > 1000)
    {
        digitalWrite(cansat.config.PARACHUTE_MOSFET_2, LOW);
    }
}

// MAIN FUNCTIONS
// Ascent state loop
bool ascent_state_loop(Cansat &cansat)
{
    unsigned long loop_start = millis();

    // Reset watchdog timer
    watchdog_update();

    String incoming_msg = cansat.receive_command(cansat);
    // Check if should reset to PREP state
    if (incoming_msg == cansat.config.RESET_STATE_MSG)
    {
        cansat.config.last_state_variables.last_state = 0;
        cansat.save_last_state(cansat);
    }
    else if (incoming_msg == cansat.config.DATA_REQUEST_MSG)
    {
        data_request = true;
    }
    else if (incoming_msg == cansat.config.RANGING_REQUEST_MSG)
    {
        ranging_request = true;
    }
    else if (incoming_msg == cansat.config.TOGGLE_MOSFET_1_MSG)
    {
        cansat.log.send_info("ACK:" + incoming_msg, true, false, false);
        toggle_mosfet_1 = true;
    }
    else if (incoming_msg == cansat.config.TOGGLE_MOSFET_2_MSG)
    {
        cansat.log.send_info("ACK:" + incoming_msg, true, false, false);
        toggle_mosfet_2 = true;
    }

    // FOR TESTING PURPOSES
    if (incoming_msg == "set_descent")
    {
        cansat.log.send_info("ACK:" + incoming_msg, true, false, false);
        return true;
    }

    // Reset watchdog timer
    watchdog_update();

    // Read data from sensors
    cansat.sensors.read_data(cansat.log, cansat.config);
    cansat.log.send_data(cansat.sensors.loggable_packet, false, true, false);
    // Reset watchdog timer
    watchdog_update();

    process_requests_ascent(cansat);

    // Reset watchdog timer
    watchdog_update();

    // Check if a sensor has failed and a restart is required
    // cansat.check_if_should_restart(cansat);

    // Save data to last state
    if (millis() - last_state_save_time_ascent >= cansat.config.ASCENT_STATE_SAVE_UPDATE_INTERVAL)
    {
        cansat.log.send_data(cansat.sensors.loggable_packet, false, false, true);
        cansat.config.last_state_variables.last_inner_temp = cansat.sensors.data.average_inner_temp;
        cansat.config.last_state_variables.last_integral_term = cansat.sensors._temp_manager->_integral_term;
        cansat.config.last_state_variables.last_safe_temp = cansat.sensors._temp_manager->_safe_temp;
        cansat.save_last_state(cansat);
        last_state_save_time_ascent = millis();
    }

    if (!cansat.sensors.read_switch_state(cansat.config))
    {
        launch_rail_removed_cycle_count++;
        if (launch_rail_removed_cycle_count >= 10)
        {
            cansat.log.send_info("Launch rail removed");
            // return true;
        }
    }
    else
    {
        launch_rail_removed_cycle_count = 0;
    }
    
    // Check if should wait before starting next loop
    unsigned long loop_time = millis() - loop_start;
    if (loop_time < cansat.config.MAX_LOOP_TIME)
    {
        delay(cansat.config.MAX_LOOP_TIME - loop_time);
    }
    return false;
}

// Ascent state setup
void ascent_state(Cansat &cansat)
{
    // DONT TOUCH THIS DELAY
    // PLEASE PLEASE DONT
    // VERY IMPORTANT
    // LORA NO WORK WITHOUT IT
    // DONT ASK WHY
    // vvvvvvvv
    delay(1000);

    pinMode(cansat.config.PARACHUTE_MOSFET_1, OUTPUT_12MA);
    pinMode(cansat.config.PARACHUTE_MOSFET_2, OUTPUT_12MA);

    // If payload has recovered to ascent state
    if (cansat._has_recovered_to_state && cansat.config.last_state_variables.last_state == 1)
    {
        // Reset watchdog timer
        watchdog_update();

        // Init sensors
        cansat.sensors.init(cansat.log, cansat.config);
        // Reset watchdog timer
        watchdog_update();

        cansat.log.send_info("Reset done");
    }

    // Reset watchdog timer
    watchdog_update();

    // Run prepare loop while waiting for arming signal
    while (!ascent_state_loop(cansat))
    {
    }
}
