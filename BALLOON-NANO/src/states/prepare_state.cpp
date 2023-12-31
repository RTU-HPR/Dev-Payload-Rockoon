#include "states/prepare_state.h"
#include <SDFS.h>
#include "hardware/watchdog.h"

unsigned long int last_state_save_time_prepare = 0;

// HELPER FUNCTIONS
// Reset last state variables to 0
void reset_last_state_values(Cansat &cansat)
{
    cansat.config.last_state_variables.last_state = 0;
    cansat.config.last_state_variables.last_log_file_index = 0;
    cansat.config.last_state_variables.last_inner_temp = 0;
    cansat.config.last_state_variables.last_integral_term = 0;
    cansat.config.last_state_variables.last_safe_temp = 0;

    cansat.config.last_state_variables.inner_temp_probe_restarted = 0;
    cansat.config.last_state_variables.imu_restarted = 0;
    
    cansat.save_last_state(cansat);
}

// Reset last state failed sensors to not failed
void reset_sensor_last_state_values(Cansat &cansat)
{
    cansat.config.last_state_variables.outer_baro_failed = 0;
    cansat.config.last_state_variables.inner_baro_failed = 0;
    cansat.config.last_state_variables.inner_temp_probe_failed = 0;
    cansat.config.last_state_variables.imu_failed = 0;
    cansat.config.last_state_variables.outer_thermistor_failed = 0;

    cansat.save_last_state(cansat);
}


// MAIN FUNCTIONS
// Prepare state loop
bool prepare_state_loop(Cansat &cansat)
{
    unsigned long loop_start = millis();

    // Reset watchdog timer
    watchdog_update();
    
    // Get any incoming message
    String incoming_msg = cansat.receive_command(cansat);

    // Check if should send telemetry data for a short moment
    if (incoming_msg == cansat.config.DATA_SEND_MSG)
    {
        cansat.log.send_info("ACK:" + incoming_msg, true, false, false);
        unsigned long data_send_start_time = millis();
        unsigned long data_send_end_time = data_send_start_time + 100000000;  // 30 seconds

        while (millis() <= data_send_end_time)
        {
            unsigned long data_send_loop_start = millis();

            // Reset watchdog timer
            watchdog_update();

            // Check for any commands from PC or LoRa
            incoming_msg = cansat.receive_command(cansat);
            if (incoming_msg == cansat.config.DATA_SEND_STOP_MSG)
            {
                break;
            }

            // Reset watchdog timer
            watchdog_update();

            // Get sensor data
            cansat.sensors.read_data(cansat.log, cansat.config);
            
            // Reset watchdog timer
            watchdog_update();
    
            cansat.log.send_data(cansat.sensors.sendable_packet, cansat.sensors.loggable_packet, true, true, true);
            
            // Reset watchdog timer
            watchdog_update();
    
            // Check if should wait before next loop
            unsigned long data_send_loop_time = millis() - data_send_loop_start;
            if (data_send_loop_time < cansat.config.MAX_LOOP_TIME)
            {
                delay(cansat.config.MAX_LOOP_TIME - data_send_loop_time);
            }
        }
    }
    // Check if should enable heater
    else if (incoming_msg == cansat.config.HEATER_ENABLE_MSG)
    {
        cansat.log.send_info("ACK:" + incoming_msg, true, false, false);
        cansat.sensors._temp_manager->_heater_enabled = true;
        cansat.sensors.set_heater(true);
    }
    // Check if should arm
    else if (incoming_msg == cansat.config.ARM_MSG)
    {
        cansat.log.send_info("ACK:" + incoming_msg, true, false, false);
        return true;
    }
    // Check if should format SD card
    else if (incoming_msg == cansat.config.FORMAT_MSG)
    {
        cansat.log.send_info("ACK:" + incoming_msg, true, false, false);
        if (cansat.log.format_storage(cansat.config))
        {
            cansat.log.send_info("Formatting done");
        }
        else
        {
            cansat.log.send_info("Formatting fail");
        }
    }
    // Check if should reset eeprom values
    else if (incoming_msg == cansat.config.RESET_STATE_MSG)
    {
        cansat.log.send_info("ACK:" + incoming_msg, true, false, false);
        reset_last_state_values(cansat);
    }
    // Check if should reset eeprom values
    else if (incoming_msg == cansat.config.RESET_SENSOR_STATES_MSG)
    {
        cansat.log.send_info("ACK:" + incoming_msg, true, false, false);
        reset_sensor_last_state_values(cansat);
    }
    // If message doesn't match any case
    else if (incoming_msg != "")
    {
        String noise_msg = "NOISE received: " + incoming_msg;
        cansat.log.send_info(noise_msg, true, false, false);
    }
    
    // Reset watchdog timer
    watchdog_update();

    // Read sensor data
    cansat.sensors.read_data(cansat.log, cansat.config);
    
    cansat.log.send_data(cansat.sensors.loggable_packet, false, true, false);

    // Reset watchdog timer
    watchdog_update();

    // Check if a sensor has failed and a restart is required
    //cansat.check_if_should_restart(cansat);
    
    // Save last state variables
    if (millis() - last_state_save_time_prepare >= cansat.config.PREPARE_STATE_SAVE_UPDATE_INTERVAL)
    {
        cansat.log.send_data(cansat.sensors.loggable_packet, false, false, true);
        cansat.save_last_state(cansat);
        last_state_save_time_prepare = millis();
    }
    
    // Check if should wait before next loop
    unsigned long loop_time = millis() - loop_start;
    if (loop_time < cansat.config.MAX_LOOP_TIME)
    {
        // Serial.println("Waiting " + String(cansat.config.MAX_LOOP_TIME - loop_time));
        delay(cansat.config.MAX_LOOP_TIME - loop_time);
    }
    return false;
}

// Prepare state setup
void prepare_state(Cansat &cansat)
{    
    // DONT TOUCH THIS DELAY
    // PLEASE PLEASE DONT
    // VERY IMPORTANT
    // LORA NO WORK WITHOUT IT
    // DONT ASK WHY
    // vvvvvvvv
    delay(1000);

    // Reset watchdog timer
    watchdog_update();
    // Init sensors
    cansat.sensors.init(cansat.log, cansat.config);

    // Reset watchdog timer
    watchdog_update();

    cansat.log.send_info("Init done");

    // Run prepare loop while waiting for arming signal
    while (!prepare_state_loop(cansat))
    {
    }
}
