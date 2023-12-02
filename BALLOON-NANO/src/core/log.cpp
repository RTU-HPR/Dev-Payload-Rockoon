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

    if (!_flash->setConfig(sd_config))
    {
        Serial.println("Config not set");
    }

    // Initialize flash
    if (!_flash->begin())
    {
        send_info("FileSystem init error");
        return;
    }
    _flash_initialized = true;
    Serial.println("Flash init success");

    init_flash_files(config);
}

void Log::init_com_lora(Config &config)
{
    _com_lora = new RadioLib_Wrapper<radio_module>(config.com_config);

<<<<<<< HEAD
=======
    // Convert the lambda function to a function pointer and pass it to set_error_output_function
    // to implement this read this: https://stackoverflow.com/questions/45386009/assign-function-pointer-inside-class
    //_com_lora->set_error_output_function(send_error);

>>>>>>> 704cc1ccddd66fd41ef68aa91ad572443cd57807
    if (!_com_lora->configure_radio(config.com_config))
    {
        Serial.println("Configuring LoRa failed");
        return;
    }
<<<<<<< HEAD
    // _com_lora->test_transmit();
=======
    _com_lora->test_transmit();
>>>>>>> 704cc1ccddd66fd41ef68aa91ad572443cd57807
    send_info("Lora init success");
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
            send_info("Failed opening file: " + String(file_name));
            return;
        }
        file.println(msg);
        file.close();
    }
}
// Sends the provided message using LoRa
<<<<<<< HEAD
bool Log::send_com_lora(String msg, bool retry_till_sent)
=======
bool Log::send_com_lora((String msg, bool retry_till_sent = false)
>>>>>>> 704cc1ccddd66fd41ef68aa91ad572443cd57807
{
    //Serial.println("Before checksum: " + String(msg));
    _com_lora->add_checksum(msg);
    //Serial.println("After checksum: " + String(msg));

    if (retry_till_sent)
    {
        //Serial.println("Trying to send");
        unsigned int start_time = millis();
        bool timeout = false;
        while (!_com_lora->transmit(msg) && !timeout)
        {
            delay(5);
<<<<<<< HEAD
            if (millis() > start_time + 1000)
=======
            if (millis() > start_time + 2000)
>>>>>>> 704cc1ccddd66fd41ef68aa91ad572443cd57807
            {
                timeout = true;
            }
        }
        return !timeout;
    }
    else
    {
        bool status = _com_lora->transmit(msg);
        return status;
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
    }
}

// Call flash, eeprom and LoRa init
void Log::init(Config &config)
{
    // Init SD card
    init_flash(config);
    init_com_lora(config);
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

    if (!_com_lora->check_checksum(msg))
    {
        send_info("Message checksum fail:" + msg);
    }
    send_info("Message received: " + msg);
}

// Sends a message over LoRa and logs the message to the info file
void Log::send_info(String msg)
{
    send_info(msg, true, true, true);
}
void Log::send_info(String msg, bool log_to_lora, bool log_to_flash, bool log_to_pc)
{
    if (log_to_pc)
    {
        // Prints message to serial
        Serial.println("! " + msg);
    }
    if (log_to_lora)
    {
        if (!send_com_lora(msg, true))
        {
            // failed sending lora msg mybe error
        }
    }
    if (log_to_flash)
    {
        // Log data to info file
        msg = String(millis()) + "," + msg;
        write_to_file(msg, _info_log_file_path_final);
    }
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
}

void Log::send_data(String sendable_packet, String loggable_packet, bool lora, bool flash, bool pc)
{
    if (log_to_pc)
    {
        // Prints message to serial
        Serial.println("!!! " + msg);
    }
    if (log_to_lora)
    {
        if (!send_com_lora(msg, true))
        {
            // failed sending lora msg mybe error
        }
    }
    if (log_to_flash)
    {
        // Log data to info file
        msg = String(millis()) + "," + msg;
        write_to_file(msg, _error_log_file_path_final);
    }
}

void Log::send_data(String sendable_packet, String loggable_packet)
{
    send_data(sendable_packet, loggable_packet, true, true, true);
}
void Log::send_data(String sendable_packet, String loggable_packet, bool log_to_lora, bool log_to_flash, bool log_to_pc)
{
    if (log_to_lora)
    {
        send_com_lora(sendable_packet);
    }
    if (log_to_flash)
    {
        write_to_file(loggable_packet, _telemetry_log_file_path_final);
    }
    if (log_to_pc)
    {
        Serial.print("/*");
        Serial.print(loggable_packet);
        Serial.println("*/");
    }
}
