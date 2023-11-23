#include "states/descent_state.h"

unsigned long state_start_time = 0;
unsigned long int last_data_transmit_time_descent = 0;
unsigned long int last_state_save_time_descent = 0;

unsigned long int last_buzzer_state_change_time = 0;
bool buzzer_state = false;

unsigned long parachute_ejection_time;
bool mosfet_on_triggered = false;
unsigned long mosfet_turnoff_time;

// HELPER FUNCTIONS
void send_data_descent(Cansat &cansat)
{
    // Print data to serial
    cansat.log.log_telemetry_data_to_pc();
    // Save data to telemetry file
    cansat.log.log_telemetry_data();

    // Check if data should be sent over LoRa
    if (millis() - last_data_transmit_time_descent >= cansat.config.LORA_DATAPACKET_COOLDOWN_DESCENT)
    {
        // Send data by LoRa
        cansat.log.transmit_data(cansat.config);
        last_data_transmit_time_descent = millis();
    }
}


// MAIN FUNCTIONS
// Descent state loop
bool descent_state_loop(Cansat &cansat)
{
    unsigned long loop_start = millis();

    // Reset watchdog timer
    watchdog_update();

    // Check if parachute deployment mosfets should be turned on
    if (millis() >= parachute_ejection_time && !mosfet_on_triggered)
    {
        digitalWrite(cansat.config.PARACHUTE_MOSFET_1, HIGH);
        digitalWrite(cansat.config.PARACHUTE_MOSFET_2, HIGH);
        mosfet_on_triggered = true;
    }
    // Check if parachute deployment mosfets should be turned off
    if (millis() >= mosfet_turnoff_time)
    {
        digitalWrite(cansat.config.PARACHUTE_MOSFET_1, LOW);
        digitalWrite(cansat.config.PARACHUTE_MOSFET_2, LOW);
    }

    // Change buzzer state
    if (millis() - last_buzzer_state_change_time > cansat.config.BUZZER_INTERVAL)
    {
        buzzer_state = !buzzer_state;
        cansat.sensors.set_buzzer(cansat.config, buzzer_state);
    }

    // Check for any commands
    String incoming_msg = cansat.receive_command(cansat);
    // Check if should reset to PREP state
    if (incoming_msg == cansat.config.RESET_STATE_MSG)
    {
        cansat.config.last_state_variables.last_state = 0;
        cansat.save_last_state(cansat);
    }
    
    // Reset watchdog timer
    watchdog_update();
    
    // Read sensor data
    cansat.sensors.read_data(cansat.log, cansat.config);
    
    // Reset watchdog timer
    watchdog_update();
    
    // Send sensor data
    send_data_descent(cansat);

    // Reset watchdog timer
    watchdog_update();

    // Check if a sensor has failed and a restart is required
    cansat.check_if_should_restart(cansat);

    // Save last state variables
    if (millis() - last_state_save_time_descent >= cansat.config.DESCENT_STATE_SAVE_UPDATE_INTERVAL)
    {
        cansat.config.last_state_variables.last_inner_temp = cansat.sensors.data.average_inner_temp;
        cansat.config.last_state_variables.last_integral_term = cansat.sensors._temp_manager->_integral_term;
        cansat.config.last_state_variables.last_safe_temp = cansat.sensors._temp_manager->_safe_temp;
        cansat.save_last_state(cansat);
    }

    // Check if should wait before next loop
    unsigned long loop_time = millis() - loop_start;
    if (loop_time < cansat.config.MAX_LOOP_TIME)
    {
        delay(cansat.config.MAX_LOOP_TIME - loop_time);
    }
    return false;
}

// Descent state setup
void descent_state(Cansat &cansat)
{
    state_start_time = millis();
 
    // If payload has recovered to descent state
    if (cansat._has_recovered_to_state && cansat.config.last_state_variables.last_state == 2)
    {
        // Reset watchdog timer
        watchdog_update();

        // Init sensors
        String status = String("Sensor status: ") + cansat.sensors.init(cansat.log, cansat.config);
        // Reset watchdog timer
        watchdog_update();

        cansat.log.send_info(status, cansat.config);
        cansat.log.send_info("Reset done", cansat.config);
    }

    parachute_ejection_time = state_start_time + cansat.config.TIME_FROM_LAUNCH_TO_EJECT;
    mosfet_turnoff_time = parachute_ejection_time + cansat.config.MOSFET_ON_TIME;

    // Reset watchdog timer
    watchdog_update();

    // CURRENTLY IT WILL NOT EXIT THIS LOOP AUTOMATICALY
    // COMMAND FROM BASE STATION HAS TO BE SENT TO RESET THE STATE
    // AND THEN PICO HAS TO BE RESTARTED
    while (!descent_state_loop(cansat))
    {
    }
}
