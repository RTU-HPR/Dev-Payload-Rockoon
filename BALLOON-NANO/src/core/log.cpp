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
        send_error("Config not set");
    }

    // Initialize flash
    if (!_flash->begin())
    {
        send_error("FileSystem init error");
        return;
    }
    _flash_initialized = true;
    send_info("Flash init success");

    init_flash_files();
}
void Log::init_com_lora(Config &config)
{
    _com_lora = new RadioLib_Wrapper<radio_module>(config.com_config);

    if (_com_lora->configure_radio(config.com_config))
    {
        send_error("Configuring LoRa failed");
        return;
    }
    _com_lora->test_transmit();
    send_info("Lora init success")
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
            send_error("Failed opening file: " + String(file_name));
            return;
        }
        file.println(msg);
        file.close();
    }
}
// Sends the provided message using LoRa
bool Log::send_com_lora(String msg, bool retry_till_sent = false)
{
    _com_lora->add_checksum(msg);

    if (retry_till_sent)
    {
        unsigned int start_time = millis();
        bool timeout = false;
        while (!_com_lora->transmit(msg) || !timeout))
            {
                delay(5);
                if (millis() > start_time + 5000)
                {
                    timeout = true;
                }
            }
        return !timeout;
    }
    else
    {
        return _com_lora->transmit(msg);
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
                send_error("Failed opening telemetry file");
                return;
            }
            if (!error_file)
            {
                send_error("Failed opening error file");
                return;
            }
            if (!info_file)
            {
                send_error("Failed opening info file");
                return;
            }

            telemetry_file.println(config.TELEMETRY_HEADER);
            info_file.println(config.INFO_HEADER);
            error_file.println(config.ERROR_HEADER);

            telemetry_file.close();
            error_file.close();
            info_file.close();
        }
    }
}

// Call flash, eeprom and LoRa init
void Log::init(Config &config)
{
    // Init SD card
    init_flash(config);
    init_lora(config);
    // Send info about files to base station
    send_info("Telemetry path: " + _telemetry_log_file_path_final);
    send_info("Info path: " + _info_log_file_path_final);
    send_info("Error path: " + _error_log_file_path_final);
}

// Checks if LoRa has received any messages. Sets the message to the received one, or to empty string otherwise
void Log::receive_com_lora(String &msg, float &rssi, float &snr)
{
    // Get data from LoRa
    if (!_com_lora->receive(msg, rssi, snr))
    {
        return;
    }

    if (!_com_lora->check_checksum(msg);)
    {
        send_info("Message checksum fail:" + msg;);
    }
    send_info("Message received: " + msg);
}

// Sends a message over LoRa and logs the message to the info file
void Log::send_info(String msg)
{
    // Prints message to serial
    Serial.println("! " + msg);

    if (!send_com_lora(msg, true))
    {
        // failed sending lora msg mybe error
    }

    // Log data to info file
    msg = String(millis()) + "," + msg;
    write_to_file(msg, _info_log_file_path_final);
}

// Sends a message over LoRa and logs the message to the error file
void Log::send_error(String msg)
{
    // Prints message to serial
    Serial.println("!!! " + msg);

    if (!send_com_lora(msg, true))
    {
        // failed sending lora msg mybe error
    }

    // Log data to error file
    msg = String(millis()) + "," + msg;
    write_to_file(msg, _error_log_file_path_final);
}

void Log::send_data(String sendable_packet, String loggable_packet, bool lora = true, bool flash = true, bool pc = true)
{
    if (lora)
    {
        send_com_lora(sendable_packet);
    }
    if (flash)
    {
        write_to_file(loggable_packet, _telemetry_log_file_path_final);
    }
    if (pc)
    {
        Serial.print("/*");
        Serial.print(loggable_packet);
        Serial.println("*/");
    }
}
