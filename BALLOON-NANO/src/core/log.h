#pragma once
#include <SPI.h>
#include <RadioLib_wrapper.h>
#include <Arduino.h>
#include "config.h"

class Log
{
public:
    FS *_flash;
    bool _flash_initialized = false;
    int _log_file_name_nr = 0;

private:
    RadioLib_Wrapper<radio_module> *_com_lora;

    bool _header_required = true;

    String _telemetry_log_file_path_final;
    String _info_log_file_path_final;
    String _error_log_file_path_final;

    void init_flash(Config &config);
    void init_com_lora(Config &config);
    void write_to_file(String msg, String file_name);
    bool send_com_lora(String msg, bool retry_till_sent = false);

public:
    void init(Config &config);
    bool format_storage(Config &config);
    void init_flash_files(Config &config);

    void send_info(String msg);
    void send_error(String msg);
    void send_data(String sendable_packet, String loggable_packet, bool lora = true, bool flash = true, bool pc = true);

    void receive_com_lora(String &msg, float &rssi, float &snr);
};
