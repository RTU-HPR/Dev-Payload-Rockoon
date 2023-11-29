#include "core/log.h"
#include <SDFS.h>

// PRIVATE FUNCTIONS
// Initialize SD card
void Log::init_flash(Config &config)
{
    // Check if SD card should even be initialized
    if (!config.LOG_TO_STORAGE)
    {
        return;
    }

    // Config
    _flash = config.FILE_SYSTEM;
    SDFSConfig sd_config;
    sd_config.setCSPin(config.SD_CARD_CS);
    sd_config.setSPI(*config.SD_CARD_SPI);

    if (_flash->setConfig(sd_config))
    {
        Serial.println("Config set");
    }
    else
    {
        Serial.println("Config not set");
    }

    // Initialize flash
    if (_flash->begin())
    {
        Serial.println("FileSystem init success");
    }
    else
    {
        Serial.println("FileSystem init error");
        return;
    }

    if (!_flash->exists(config.LAST_STATE_VARIABLE_FILE_NAME))
    {
        File file = _flash->open(config.LAST_STATE_VARIABLE_FILE_NAME, "w");
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
        file.println(data);
        file.close();
        Serial.println("Created last save state file");
    }

    // FSInfo64 fsinfo;
    //_flash->info64(fsinfo);
    // Serial.println("Current size:" + String((unsigned long)fsinfo.usedBytes / 1024) + "/" + String((unsigned long)fsinfo.totalBytes / 1024));

    _flash_initialized = true;
}

// Writes a given message to a file on the SD card
void Log::write_to_file(String msg, String file_name)
{
    // Write info to SD card
    if (_flash_initialized)
    {
        // write to flash
        File file = _flash->open(file_name, "a+");
        if (!file)
        {
            Serial.println("Failed opening file: " + String(file_name));
            return;
        }
        file.println(msg);
        file.close();
    }
}

// PUBLIC FUNCTIONS
// Format SD card
bool Log::format_storage(Config &config)
{
    bool result = _flash->format();
    init_flash(config);
    return result;
}

// Open (create) telemetry/info/error files
void Log::init_flash_files(Config &config)
{
    if (_flash_initialized)
    {
        // Determine file name index for final path
        // Create a new file, if no log file to recover to
        if (config.last_state_variables.last_state == 0)
        {
            while (_flash->exists(config.TELEMETRY_LOG_FILE_NAME_BASE_PATH + String(_log_file_name_nr) + ".csv"))
            {
                _log_file_name_nr++;
            }
        }
        // If recovering to existing file, no header will be required
        else
        {
            _header_required = false;
        }

        // Get the file full path
        _telemetry_log_file_path_final = config.TELEMETRY_LOG_FILE_NAME_BASE_PATH + String(_log_file_name_nr) + ".csv";
        _info_log_file_path_final = config.INFO_LOG_FILE_NAME_BASE_PATH + String(_log_file_name_nr) + ".csv";
        _error_log_file_path_final = config.ERROR_LOG_FILE_NAME_BASE_PATH + String(_log_file_name_nr) + ".csv";

        // If required, write the file header
        if (_header_required)
        {
            File telemetry_file = _flash->open(_telemetry_log_file_path_final, "a+");
            File error_file = _flash->open(_error_log_file_path_final, "a+");
            File info_file = _flash->open(_info_log_file_path_final, "a+");

            if (!telemetry_file)
            {
                Serial.println("Failed opening telemetry file");
                return;
            }
            if (!error_file)
            {
                Serial.println("Failed opening error file");
                return;
            }
            if (!info_file)
            {
                Serial.println("Failed opening info file");
                return;
            }

            telemetry_file.println(config.TELEMETRY_HEADER);
            info_file.println(config.INFO_HEADER);
            error_file.println(config.ERROR_HEADER);

            telemetry_file.close();
            error_file.close();
            info_file.close();
        }

        // Send info about files to base station
        send_com_lora("Telemetry path: " + _telemetry_log_file_path_final);
        send_com_lora("Info path: " + _info_log_file_path_final);
        send_com_lora("Error path: " + _error_log_file_path_final);
    }
}

// Call flash, eeprom and LoRa init
void Log::init(Config &config)
{
    // Init SD card
    init_flash(config);

    // Init LoRa
    _com_lora = new RadioLib_Wrapper<radio_module>(config.com_config);
    
    bool status = _com_lora->configure_radio(config.com_config);
    if (!status)
    {
        String msg = "Configuring LoRa failed";
        log_error_msg_to_flash(msg);
        Serial.println(msg);
    }
    _com_lora->test_transmit();
}

// Sends the provided message using LoRa
bool Log::send_com_lora(String msg)
{
    _com_lora->add_checksum(msg);
    return _com_lora->transmit(msg);
}

// Checks if LoRa has received any messages. Sets the message to the received one, or to empty string otherwise
void Log::receive_com_lora(String &msg, float &rssi, float &snr)
{
    // Get data from LoRa
    if (!_com_lora->receive(msg, rssi, snr))
    {
        return;
    }
    
    bool status = _com_lora->check_checksum(msg);
    if (!status)
    {
        String error_msg = "Message checksum fail";
        Serial.println(error_msg);
        log_info_msg_to_flash(error_msg);
    }
    Serial.println("Message received: " + msg);
}

// Sends a message over LoRa and logs the message to the info file
void Log::send_info(String msg)
{
    // Prints message to serial
    Serial.println("! " + msg);

    int state = send_com_lora(msg);
    if (state != RADIOLIB_ERR_NONE)
    {
        Serial.println("Transmit error: " + String(state));
    }
    // Log data to info file
    msg = String(millis()) + "," + msg;
    log_info_msg_to_flash(msg);
}

// Sends a message over LoRa and logs the message to the error file
void Log::send_error(String msg)
{
    // Prints message to serial
    Serial.println("!!! " + msg);

    int state = send_com_lora(msg);
    if (state != RADIOLIB_ERR_NONE)
    {
        Serial.println("Transmit error: " + String(state));
    }
    // Log data to error file
    msg = String(millis()) + "," + msg;
    log_error_msg_to_flash(msg);
}

// Send telemetry data packet over LoRa
void Log::transmit_data()
{
    int state = send_com_lora(_sendable_packet);
    if (state != RADIOLIB_ERR_NONE)
    {
        Serial.println("Transmit error: " + String(state));
    }
}

// Logs the full data packet to SD card
void Log::log_telemetry_data()
{
    write_to_file(_loggable_packet, _telemetry_log_file_path_final);
}

// Sends the full data packet to Serial port
void Log::log_telemetry_data_to_pc()
{
    Serial.print("/*");
    // Serial.print(sendable_packet);
    Serial.print(_loggable_packet);
    Serial.println("*/");
}

// Logs the provided info message to info file on SD card
void Log::log_info_msg_to_flash(String msg)
{
    write_to_file(msg, _info_log_file_path_final);
}

// Logs the provided error message to error file on SD card
void Log::log_error_msg_to_flash(String msg)
{
    write_to_file(msg, _error_log_file_path_final);
}