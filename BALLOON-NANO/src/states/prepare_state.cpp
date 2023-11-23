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
    
    String incoming_msg = cansat.receive_command(cansat);
    
    // Reset watchdog timer
    watchdog_update();
    // Read sensor data
    cansat.sensors.read_data(cansat.log, cansat.config);

    // Reset watchdog timer
    watchdog_update();

    // Save data to telemetry file
    cansat.log.log_telemetry_data();

    // Reset watchdog timer
    watchdog_update();

    // Check if a sensor has failed and a restart is required
    cansat.check_if_should_restart(cansat);
    
    // Save last state variables
    if (millis() - last_state_save_time_prepare >= cansat.config.PREPARE_STATE_SAVE_UPDATE_INTERVAL)
    {
        cansat.save_last_state(cansat);
    }
    
    // Check received message
    // Check if should send telemetry data for a short moment
    if (incoming_msg == cansat.config.DATA_SEND_MSG)
    {
        unsigned long data_send_start_time = millis();
        unsigned long data_send_end_time = data_send_start_time + 30000;  // 30 seconds 
        while (millis() <= data_send_end_time)
        {
            unsigned long data_send_loop_start = millis();
            // Check for any commands from PC or LoRa
            incoming_msg = cansat.receive_command(cansat);
            
            // Reset watchdog timer
            watchdog_update();
    
            if (incoming_msg == cansat.config.DATA_SEND_STOP_MSG)
            {
                break;
            }

            // Get sensor data
            cansat.sensors.read_data(cansat.log, cansat.config);
            
            // Reset watchdog timer
            watchdog_update();
    
            // Print data to serial
            cansat.log.log_telemetry_data_to_pc();
            // Save data to telemetry file
            cansat.log.log_telemetry_data();
            // Send data by LoRa
            cansat.log.transmit_data(cansat.config);
            
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
        cansat.sensors._temp_manager->_heater_enabled = true;
        cansat.sensors.set_heater(true);
    }
    // Check if should arm
    else if (incoming_msg == cansat.config.ARM_MSG)
    {
        cansat.log.send_info("Arming signal received", cansat.config);
        return true;
    }
    // Check if should format SD card
    else if (incoming_msg == cansat.config.FORMAT_MSG)
    {
        if (cansat.log.format_storage(cansat.config))
        {
            cansat.log.send_info("Formatting done", cansat.config);
        }
        else
        {
            cansat.log.send_info("Formatting fail", cansat.config);
        }
    }
    // Check if should reset eeprom values
    else if (incoming_msg == cansat.config.RESET_EEPROM_MSG)
    {
        reset_last_state_values(cansat);
    }
    // Check if should reset eeprom values
    else if (incoming_msg == cansat.config.RESET_SENSOR_STATES_MSG)
    {
        reset_sensor_last_state_values(cansat);
    }
    // If message doesn't match any case
    else if (incoming_msg != "")
    {
        String noise_msg = "NOISE received: " + incoming_msg;
        cansat.log.send_info(noise_msg, cansat.config);
    }
    
    // Reset watchdog timer
    watchdog_update();
    
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
    // Reset watchdog timer
    watchdog_update();

    // Init sensors
    String status = String("Sensor status: ") + cansat.sensors.init(cansat.log, cansat.config);

    // Reset watchdog timer
    watchdog_update();

    cansat.log.send_info(status, cansat.config);

    cansat.log.send_info("Init done, waiting for arm", cansat.config);

    // Run prepare loop while waiting for arming signal
    while (!prepare_state_loop(cansat))
    {
    }
}
