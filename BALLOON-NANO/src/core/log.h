#pragma once
#include <SPI.h>
#include <RadioLib.h>
#include <Arduino.h>
#include "config.h"

class Log
{
private:
    RFM_Wrapper _com_lora;
    String _telemetry_log_file_path_final;
    String _info_log_file_path_final;
    String _error_log_file_path_final;

    void init_flash(Config &config);
    void write_to_file(String msg, String file_name);

public:
    FS *_flash;
    bool _flash_initialized = false;

    String _loggable_packet;
    String _sendable_packet;

    int _log_file_name_nr = 0;
    bool _header_required = true;

    void init(Config &config);
    void init_flash_files(Config &config);
    bool format_storage(Config &config);
    
    void send_info(String msg, Config &config);
    void send_error(String msg, Config &config);
    
    bool send_com_lora(String msg, Config &config);
    void receive_com_lora(String &msg, float &rssi, float &snr, Config &config);

    void transmit_data(Config &config);
    void log_telemetry_data();
    void log_telemetry_data_to_pc();

    void log_info_msg_to_flash(String msg);
    void log_error_msg_to_flash(String msg);
};
