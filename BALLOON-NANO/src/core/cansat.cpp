#include <core/cansat.h>

// PRIVATE FUNCTIONS
// Initialize SPI/I2C/PC Serial/Hardware Serial communication
void Cansat::init_all_com_bus(Config &config)
{
    SPI.setRX(config.SPI0_RX);
    SPI.setTX(config.SPI0_TX);
    SPI.setSCK(config.SPI0_SCK);
    SPI.begin();

    SPI1.setRX(config.SPI1_RX);
    SPI1.setTX(config.SPI1_TX);
    SPI1.setSCK(config.SPI1_SCK);
    SPI1.begin();

    Wire.setSCL(config.WIRE0_SCL);
    Wire.setSDA(config.WIRE0_SDA);

    // Wire1.setSCL(config.WIRE1_SCL);
    // Wire1.setSDA(config.WIRE1_SDA);

    // Initialize PC serial
    /*
    Serial.begin(config.PC_BAUDRATE);
    if (config.WAIT_PC)
    {
        while (!Serial)
        {
            delay(500);
        }
    }
    */
    Serial1.setRX(config.SERIAL1_RX);
    Serial1.setTX(config.SERIAL1_TX);
    Serial1.setFIFOSize(256); // once had a problem of not reading serial properly but this seemed to fix it
    Serial1.begin(config.SERIAL1_BAUDRATE);
}

// Read last state from SD card
void Cansat::read_last_state(Cansat &cansat)
{
    if (cansat.log._flash_initialized)
    {
        // Open file
        File file = cansat.log._flash->open(cansat.config.LAST_STATE_VARIABLE_FILE_NAME, "r");
        if (!file)
        {
            Serial.println("Failed opening file: " + String(cansat.config.LAST_STATE_VARIABLE_FILE_NAME));
            return;
        }

        // Probably there is a better way to do this, but this seems to well enough
        // Read the entire csv row in a String variable
        String read_str = "";
        while (file.available())
        {
            read_str += file.readString();
        }
        // Make a char array from the string
        int read_str_len = read_str.length() + 1;
        char char_array[read_str_len];
        read_str.toCharArray(char_array, read_str_len);

        cansat.log.send_com_lora("Last state char array: " + String(char_array), cansat.config);

        // Set the last state variables to the ones read from the file
        int result = sscanf(char_array,                                     // The char array to read from
                            "%i,%i,%f,%f,%f,%i,%i,%i,%i,%i,%i,%i/n",     // The form the variables are separated in string
                            &cansat.config.last_state_variables.last_state, // The values to update that correspond to the variable in string
                            &cansat.config.last_state_variables.last_log_file_index,
                            &cansat.config.last_state_variables.last_inner_temp,
                            &cansat.config.last_state_variables.last_integral_term,
                            &cansat.config.last_state_variables.last_safe_temp,
                            &cansat.config.last_state_variables.outer_baro_failed,
                            &cansat.config.last_state_variables.inner_baro_failed,
                            &cansat.config.last_state_variables.inner_temp_probe_failed,
                            &cansat.config.last_state_variables.imu_failed,
                            &cansat.config.last_state_variables.outer_thermistor_failed,
                            &cansat.config.last_state_variables.inner_temp_probe_restarted,
                            &cansat.config.last_state_variables.imu_restarted);
        // Close the file
        file.close();
    }
}

// Sets any required variables from last state to current state
void Cansat::recover_state_variables(Cansat &cansat)
{
    cansat.log._log_file_name_nr = cansat.config.last_state_variables.last_log_file_index;
    cansat.sensors.data.average_inner_temp = cansat.config.last_state_variables.last_inner_temp;
}

// Log initialization data to flash, after SD card is set up and files opened
void Cansat::save_to_flash_after_init(Cansat &cansat)
{
    // Log the read last state variables
    cansat.log.log_info_msg_to_flash("Last state: " + String(cansat.config.last_state_variables.last_state));
    cansat.log.log_info_msg_to_flash("Last log file index: " + String(cansat.config.last_state_variables.last_log_file_index));
    cansat.log.log_info_msg_to_flash("Last inner temp: " + String(cansat.config.last_state_variables.last_inner_temp));
    cansat.log.log_info_msg_to_flash("Last integral term: " + String(cansat.config.last_state_variables.last_integral_term));
    cansat.log.log_info_msg_to_flash("Last safe temperature: " + String(cansat.config.last_state_variables.last_safe_temp));

    cansat.log.log_info_msg_to_flash("Outer baro failed: " + String(cansat.config.last_state_variables.outer_baro_failed));
    cansat.log.log_info_msg_to_flash("Inner baro failed: " + String(cansat.config.last_state_variables.inner_baro_failed));
    cansat.log.log_info_msg_to_flash("Inner temp probe failed: " + String(cansat.config.last_state_variables.inner_temp_probe_failed));
    cansat.log.log_info_msg_to_flash("IMU failed: " + String(cansat.config.last_state_variables.imu_failed));
    cansat.log.log_info_msg_to_flash("Outer thermistor failed: " + String(cansat.config.last_state_variables.outer_thermistor_failed));

    cansat.log.log_info_msg_to_flash("Inner temp probe restarted: " + String(cansat.config.last_state_variables.inner_temp_probe_restarted));
    cansat.log.log_info_msg_to_flash("IMU restarted: " + String(cansat.config.last_state_variables.imu_restarted));
}

// PUBLIC FUNCTIONS
// Receive commands from LoRa or PC
String Cansat::receive_command(Cansat &cansat)
{
    // Check for further commands either from PC or LoRa
    String incoming_msg = "";
    float rssi;
    float snr;
    cansat.log.receive_com_lora(incoming_msg, rssi, snr, cansat.config);
    if (Serial.available() > 0)
    {
        incoming_msg = Serial.readString();
    }

    // Remove any new line characters and log the received message
    if (incoming_msg != "")
    {
        incoming_msg.trim();
        Serial.println(incoming_msg + ", " + String(rssi) + ", " + String(snr));
        cansat.log.send_info("Payload received message: " + incoming_msg, cansat.config);
    }
    // Return the message
    return incoming_msg;
}   

// Write last state from SD card
void Cansat::save_last_state(Cansat &cansat)
{
    if (cansat.log._flash_initialized)
    {
        // Open file
        File file = cansat.log._flash->open(cansat.config.LAST_STATE_VARIABLE_FILE_NAME, "w");
        if (!file)
        {
            Serial.println("Failed opening file: " + String(config.LAST_STATE_VARIABLE_FILE_NAME));
            return;
        }

        // Save all the latest last state variables to string
        String data;
        data += String(config.last_state_variables.last_state);
        data += ",";
        data += String(config.last_state_variables.last_log_file_index);
        data += ",";
        data += String(config.last_state_variables.last_inner_temp);
        data += ",";
        data += String(config.last_state_variables.last_integral_term);
        data += ",";
        data += String(config.last_state_variables.last_safe_temp);
        data += ",";
        data += String(config.last_state_variables.outer_baro_failed);
        data += ",";
        data += String(config.last_state_variables.inner_baro_failed);
        data += ",";
        data += String(config.last_state_variables.inner_temp_probe_failed);
        data += ",";
        data += String(config.last_state_variables.imu_failed);
        data += ",";
        data += String(config.last_state_variables.outer_thermistor_failed);
        data += ",";
        data += String(config.last_state_variables.inner_temp_probe_restarted);
        data += ",";
        data += String(config.last_state_variables.imu_restarted);

        // Overwrite the last state variables with the new ones
        file.println(data);

        // Close the file
        file.close();
    }
}

// Cycle 3.3V power bus and restart Pico
void Cansat::restart(Cansat &cansat)
{
    // Cycle power off/on
    cansat.sensors.reset_sensor_power(cansat.config);
    // Restart Pico
    watchdog_reboot(0, 0, config.WATCHDOG_TIMER);
}

void Cansat::check_if_should_restart(Cansat &cansat)
{
    if (cansat.sensors._hard_reset_required)
    {   
        // Save which sensor has failed
        if (!cansat.sensors._imu_initialized)
        {
            cansat.config.last_state_variables.imu_failed = 1;
        }
        if (!cansat.sensors._inner_temp_probe_initialized)
        {
            cansat.config.last_state_variables.inner_temp_probe_failed = 1;
        }
        cansat.save_last_state(cansat);
        
        cansat.log.send_info("Hard reset initalized", cansat.config);
        
        // Do a restart
        cansat.restart(cansat);
    }
}

// Main function that checks and runs appropriate state
void Cansat::start_states(Cansat &cansat)
{
    // FOR DEBUGGING PURPOSES
    // IT SHOULDN'T BE HERE, IT SHOULD ONLY BE IN INIT ALL COM BUS
    Serial.begin(config.PC_BAUDRATE);
    if (config.WAIT_PC)
    {
        while (!Serial)
        {
            delay(500);
        }
    }

    // Initialize communications
    cansat.init_all_com_bus(cansat.config);

    // Initialize SD card/communication LoRa
    cansat.log.init(cansat.config);

    // Read last state data
    read_last_state(cansat);

    // Set required variables from last state
    recover_state_variables(cansat);

    // Open/create telemetry/info/error files
    cansat.log.init_flash_files(cansat.config);

    // Check if Watchdog restart has happened
    if (watchdog_caused_reboot())
    {
        cansat.log.send_info("Watchdog caused a reboot", cansat.config);
    }

    // Save the current file index to last state
    cansat.config.last_state_variables.last_log_file_index = cansat.log._log_file_name_nr;
    cansat.save_last_state(cansat);

    // Check in which state should payload start in
    int last_state = cansat.config.last_state_variables.last_state;
    cansat.log.send_info("Last state: " + String(last_state), cansat.config);

    // Enable Watchdog
    watchdog_enable(cansat.config.WATCHDOG_TIMER, 1);
    cansat.log.send_info("Watchdog is enabled with time: " + String(cansat.config.WATCHDOG_TIMER), cansat.config);

    // If ascent/descent state is not set, start in prepare state
    if (last_state == 0)
    {
        cansat.log.send_info("No previous state. Starting from PREP state", cansat.config);
        // Start prepare state
        current_state = State::PREP;
        prepare_state(cansat);

        // Start ascent state
        current_state = State::ASCENT;
        // Set the ascent state as the last state
        cansat.config.last_state_variables.last_state = 1;
        save_last_state(cansat);
        ascent_state(cansat);

        // Start descent state
        current_state = State::DESCENT;
        // Set the descent state as the last state
        cansat.config.last_state_variables.last_state = 2;
        save_last_state(cansat);
        descent_state(cansat);
    }
    // If ascent state is set as the last state
    else if (last_state == 1)
    {
        // FOR DEBUGGING PURPOSES
        cansat.config.WAIT_PC = true;

        // Set flag to know if a restart has happened
        _has_recovered_to_state = true;

        cansat.log.send_info("Previous state ASCENT. Starting from ASCENT", cansat.config);

        // Start ascent state
        current_state = State::ASCENT;
        ascent_state(cansat);

        // Start descent state
        current_state = State::DESCENT;
        // Set the descent state as the last state
        cansat.config.last_state_variables.last_state = 1;
        save_last_state(cansat);
        descent_state(cansat);
    }
    // If descent state is set as the last state
    else if (last_state == 2)
    {
        // FOR DEBUGGING PURPOSES
        cansat.config.WAIT_PC = true;

        // Set flag to know if a restart has happened
        _has_recovered_to_state = true;

        cansat.log.send_info("Previous state DESCENT. Starting from DESCENT", cansat.config);

        // Start descent state
        current_state = State::DESCENT;
        descent_state(cansat);
    }
}
