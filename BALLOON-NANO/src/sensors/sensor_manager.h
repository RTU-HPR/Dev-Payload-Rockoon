#pragma once
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <MS5611.h>
#include <Adafruit_SHTC3.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085.h>
#include <LSM6.h>
#include <SoftwareSerial.h>
#include <NTC_Thermistor.h>
#include <RadioLib.h>
#include "temperature_manager.h"
#include <ranging_wrapper.h>
#include "core/data_filtering.h"
#include <Array.h>
#include "config.h"
#include <core/log.h>
#include <PCF8575.h>

/**
 * @brief A class responsible for initializing, managing and reading data from all the different sensors and controllers. All the data is stored in the data struct
 *
 */
class Sensor_manager
{
private:
    // SENSOR OBJECTS AND Communication
    // GPS UART0
    TinyGPSPlus _gps;
    SerialUART *_gps_serial;
    unsigned long _last_gps_packet_time = 0;

    // BARO WIRE0
    MS5611 _outer_baro;
    int _outer_baro_consecutive_failed_readings = 0;

    // BARO WIRE0
    Adafruit_BMP085 _inner_baro;
    int _inner_baro_consecutive_failed_readings = 0;

    // IMU WIRE0
    LSM6 _imu;
    int _imu_consecutive_failed_readings = 0;

    // TEMPERATURE WIRE0
    ClosedCube::Sensor::STS35 _inner_temp_probe;
    int _inner_temp_probe_consecutive_failed_readings = 0;

    // TEMPERATURE NTC
    NTC_Thermistor _outer_thermistor = NTC_Thermistor(0, 0, 0, 0, 0);
    int _outer_thermistor_consecutive_failed_readings = 0;

    // Ranging lora
    Ranging_Wrapper _ranging_lora;
    unsigned long _last_ranging_pos_time = 0;
    int _last_slave_index = 0;
    int _slave_index = 0;
    int _ranging_lora_consecutive_failed_readings = 0;

    // Temperature manager
    bool _heater_enabled = false;
    bool _heater_constant = false;
    float _heater_constant_temp;
    
    // Averagers 
    Time_Averaging_Filter<float> *_inner_temp_averager;
    Time_Averaging_Filter<float> *_inner_temp_averager_baro;
    Time_Averaging_Filter<float> *_outer_temp_averager;
    Time_Averaging_Filter<float> *_batt_averager;
    Time_Averaging_Filter<float> *_heater_current_averager;

    // Battery
    int _batt_voltage_consecutive_failed_readings = 0;
    
    // Heater current
    int _heater_current_consecutive_failed_readings = 0;

    void position_calculation(Log &log, Config &config);
    void read_ranging(Log &log, Config &config);
    void read_gps(Log &log, Config &config);
    void read_outer_baro(Log &log, Config &config);
    void read_inner_baro(Log &log, Config &config);
    void read_inner_temp_probe(Log &log, Config &config);
    void read_outer_thermistor(Log &log, Config &config);
    void read_imu(Log &log, Config &config);
    void read_time();
    void update_heater(Log &log, Config &config);
    void read_batt_voltage(Log &log, Config &config);
    void read_heater_current(Log &log, Config &config);

public:
    // Temp manager
    Temperature_Manager *_temp_manager;

    // Port extender
    PCF8575 *_port_extender;
    
    bool _hard_reset_required = false;

    bool _gps_initialized = false;
    bool _outer_baro_initialized = false;
    bool _inner_baro_initialized = false;
    bool _imu_initialized = false;
    bool _inner_temp_probe_initialized = false;
    bool _outer_thermistor_initialized = false;
    bool _ranging_lora_initalized = false;

    // TODO make a different struct for sendable data and raw data
    struct Sensor_data
    {
        // array data is ordered: x y z
        float gps_lat = 0;      // deg
        float gps_lng = 0;      // deg
        float gps_height = 0;   // m
        int gps_satellites = 0; // count

        float outer_baro_pressure = 0;
        float outer_baro_temp = 0;

        float inner_baro_pressure = 0; // Pa
        float inner_baro_temp = 0;     // C

        float inner_temp_probe = 0; // C
        float outer_temp_thermistor = 0;

        float average_inner_temp = 0;  // C
        float average_inner_temp_baro = 0;
        float average_outer_temp = 0; // C
        float heater_power = 0;        // 0-255
        float p = 0;                   // proportional * coefficient
        float i = 0;                   // integral * coefficient
        float d = 0;                   // derivative * coefficient
        float target_temp = 0;

        float acc[3] = {0, 0, 0};  // m/s^2
        float gyro[3] = {0, 0, 0}; // dps

        Ranging_Wrapper::Ranging_Result ranging_results[3];
        Ranging_Wrapper::Position ranging_position = Ranging_Wrapper::Position(0, 0, 0);

        float batt_voltage = 0;         // V
        float average_batt_voltage = 0; // v

        float heater_current = 0;
        float average_heater_current = 0;

        unsigned long time = 0;                           // ms
        unsigned long time_since_last_gps = 0;            // ms
        unsigned long times_since_last_ranging_result[3]; // ms
        unsigned long time_since_last_ranging_pos = 0;    // ms
        unsigned long gps_time = 0;                       //
    };

    // Set heater state
    void set_heater(bool state)
    {
        _heater_enabled = state;
        data.heater_power = 0;
        _temp_manager->reset();
    };
    
    String header = "Data header:";
    Sensor_data data;
    
    String init(Log &log, Config &config);
    void reset_sensor_power(Config &config);
    bool read_switch_state(Config &config);
    void set_buzzer(Config &config, bool state);
    void set_status_led_1(Config &config, bool state);
    void set_status_led_2(Config &config, bool state);
    void read_data(Log &log, Config &config);
    void update_data_packet(Sensor_data &data, String &result_sent, String &result_log);
};