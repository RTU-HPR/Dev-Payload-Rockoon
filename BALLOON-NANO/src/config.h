#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <LittleFS.h>
#include <SDFS.h>
#include <ranging_wrapper.h>
#include <RadioLib_wrapper.h>

class Config
{
public:
    //----------- CHANGES DURING RUNTIME ------------------
    // I know this shouldn't be here, but this was the easiest place to put this for state saving to work
    struct Last_state_variables
    {
        int last_state = 0;
        int last_log_file_index = 0;

        float last_inner_temp = 0;
        float last_integral_term = 0;
        float last_safe_temp = 0;

        int outer_baro_failed = 0;
        int inner_baro_failed = 0;
        int imu_failed = 0;
        int inner_temp_probe_failed = 0;
        int outer_thermistor_failed = 0;

        int outer_baro_restarted = 0;
        int inner_baro_restarted = 0;
        int imu_restarted = 0;
        int inner_temp_probe_restarted = 0;
        int outer_thermistor_restarted = 0;
    };
    Last_state_variables last_state_variables;

    //------------ OFTEN CHANGED ------------------
    // hard data rate limiter
    const int MAX_LOOP_TIME = 20; // ms

    bool WAIT_PC = false;
    const bool LOG_TO_STORAGE = true;

    // 433 MHz LoRa
    #define radio_module SX1268
    RadioLib_Wrapper<radio_module>::Radio_Config com_config
    {
        .frequency = 434.5,
        .cs = 2,
        .dio0 = 3,
        .dio1 = 5,
        .family = RadioLib_Wrapper<radio_module>::Radio_Config::Chip_Family::Sx126x,
        .rf_switching = RadioLib_Wrapper<radio_module>::Radio_Config::Rf_Switching::Dio2,
        // .rx_enable = 0, // only needed if rf_switching = gpio
        // .tx_enable = 0, // only needed if rf_switching = gpio
        .reset = 8,
        .sync_word = 0xF4,
        .tx_power = 22,
        .spreading = 11,
        .coding_rate = 8,
        .signal_bw = 62.5,
        .frequency_correction = false,
        .spi_bus = &SPI,
    };

    // Ranging 2.4 GHZ LoRa
    Ranging_Wrapper::Ranging_Slave RANGING_SLAVES[3] = 
    {
        {.position = {0, 0, 0}, .address = 0x12345678},
        {.position = {0, 0, 0}, .address = 0xABCD9876},
        {.position = {0, 0, 0}, .address = 0x9A8B7C6D}
    };
    Ranging_Wrapper::Mode LORA2400_MODE = Ranging_Wrapper::Mode::MASTER;
    Ranging_Wrapper::Lora_Device ranging_device = 
    {
        .FREQUENCY = 2405.6,
        .CS = 10,
        .DIO0 = 13, // busy
        .DIO1 = 12,
        .RESET = 11,
        .SYNC_WORD = 0xF5,
        .TXPOWER = 14,
        .SPREADING = 10,
        .CODING_RATE = 7,
        .SIGNAL_BW = 406.25,
        .SPI = &SPI
    };

    const float SEA_LEVEL_PRESSURE = 101325;
    const float HEATER_CUT_OFF_VOLTAGE = 4;//5.9; // V
    const float DESIRED_HEATER_TEMP = 35.0;   // in C

    const unsigned int OUTER_TEMP_AVERAGE_TIME = 3000;
    const unsigned int OUTER_TEMP_AVERAGE_CAPACITY = ((OUTER_TEMP_AVERAGE_TIME / MAX_LOOP_TIME) * 1.5);

    const unsigned int INNER_TEMP_AVERAGE_TIME = 3000;
    const unsigned int INNER_TEMP_AVERAGE_CAPACITY = ((INNER_TEMP_AVERAGE_TIME / MAX_LOOP_TIME) * 1.5);

    const unsigned int BAT_AVERAGE_TIME = 5000;
    const unsigned int BAT_AVERAGE_CAPACITY = ((BAT_AVERAGE_TIME / MAX_LOOP_TIME) * 1.5);

    const int LORA_DATAPACKET_COOLDOWN_ASCENT = 30000; // in ms  30000
    const int LORA_DATAPACKET_COOLDOWN_DESCENT = 3000;
    const int TIME_FROM_LAUNCH_TO_EJECT = 20000; // ms
    const int MOSFET_ON_TIME = 10000;            // ms

    //----------------------------------SOMEWHAT STATIC -------------------------------------

    // State saving
    const String LAST_STATE_VARIABLE_FILE_NAME = "/CANSAT_LAST_STATE.csv";

    const unsigned int PREPARE_STATE_SAVE_UPDATE_INTERVAL = 10000;
    const unsigned int ASCENT_STATE_SAVE_UPDATE_INTERVAL = 5000;
    const unsigned int DESCENT_STATE_SAVE_UPDATE_INTERVAL = 5000;

    // Watchdog
    const int WATCHDOG_TIMER = 8000; // Max is 8400

    // Sensor power
    const int SENSOR_POWER_ENABLE_PIN = 17;

    const int LAUNCH_RAIL_SWITCH_PIN = 21; // Switch

    // I2C
    // Wire0 enable if using gps
    const int WIRE0_SCL = 1;
    const int WIRE0_SDA = 0;

    // Wire1
    const int WIRE1_SCL = 15;
    const int WIRE1_SDA = 14;

    // I2C bus usage
    // Wire1
    TwoWire *IMU_WIRE = &Wire1;
    TwoWire *STS35_WIRE = &Wire1;
    TwoWire *BMP280_WIRE = &Wire1;
    TwoWire *MS5611_WIRE = &Wire1;
    TwoWire *PORT_EXTENDER_WIRE = &Wire1;

    // I2C addresses
    const int PORT_EXTENDER_ADDRESS_I2C = 0x20;
    const int MS5611_ADDRESS_I2C = 0x76;
    const int BMP180_ADDRESS_I2C = 0x77; // or 0x76 :D
    const int IMU_ADDRESS_I2C = 0x6B;  // or 0x6A
    const int STS35_ADDRESS_I2C = 0x4B; // I2C Address: either 0x4A or 0x04B
    const int GPS_ADDRESS_I2C = 0x42;

    // SPI
    // SPI0
    const int SPI0_RX = 4; // schematic was changed from 8 -> 4
    const int SPI0_TX = 7;
    const int SPI0_SCK = 6;

    // SPI1
    // const int SPI1_RX = 12;
    // const int SPI1_TX = 11;
    // const int SPI1_SCK = 10;

    // SPI bus usage
    // Sd card
    const int SD_CARD_CS = 9;
    SPIClassRP2040 *SD_CARD_SPI = &SPI;
 
    // Port extender
    const int PORT_EXTENDER_BUZZER_PIN = 1;             // Buzzer
    const int PORT_EXTENDER_LED_2_PIN = 2;              // Status LED 2
    const int PORT_EXTENDER_LED_1_PIN = 3;              // Status LED

    // logging
    const unsigned long PC_BAUDRATE = 115200;
    FS *FILE_SYSTEM = &SDFS; // if change to LittleFS need to change some code

    // GPS UART0
    // const int SERIAL1_RX = 1;
    // const int SERIAL1_TX = 0;
    // const long SERIAL1_BAUDRATE = 9600;
    
    // OUTER TEMP ANALOG
    const int THERMISTOR_PIN = 28;
    const float THERMISTOR_REFERENCE_R = 10000;
    const float THERMISTOR_NOMINAL_R = 10000;
    const float THERMISTOR_NOMINAL_T = 25;
    const float THERMISTOR_B = -4050;

    // Battery voltage
    const float BATT_SENS_CONVERSION_FACTOR = 3.3 * 3.1251;
    const int BATT_SENS_PIN = 26;

    // HEATER
    const int HEATER_MOSFET = 16; // mosfet 1

    // Heater current
    const float HEATER_CURR_SENS_PIN = 27;
    const float HEATER_CURR_CONVERSION_FACTOR = 3.3 * 3.1251;
    const float HEATER_RESISTOR_VALUE = 1.1;

    // Parachute
    const int PARACHUTE_MOSFET_1 = 19;
    const int PARACHUTE_MOSFET_2 = 18; // mosfet 2

    // Buzzer
    const int BUZZER_INTERVAL = 300;

    // SENSOR TIMEOUT CONSTANTS (milliseconds)
    const unsigned int OUTER_BARO_TIMEOUT = 200;
    const unsigned int INNER_BARO_TIMEOUT = 200;
    const unsigned int INNER_TEMP_PROBE_TIMEOUT = 200;
    const unsigned int IMU_TIMEOUT = 200;
    const unsigned int OUTER_THERMISTOR_TIMEOUT = 200;
    const unsigned int RANGING_LORA_TIMEOUT = 500;

    // Max allowed unsuccessful readings
    const int OUTER_BARO_MAX_ATTEMPTS = 3;
    const int INNER_BARO_MAX_ATTEMPTS = 3;
    const int INNER_TEMP_PROBE_MAX_ATTEMPTS = 3;
    const int IMU_MAX_ATTEMPTS = 3;
    const int OUTER_THERMISTOR_MAX_ATTEMPTS = 3;

    // SENSOR VALUE ACCEPTABLE RANGES
    // Outer baro (Datasheet operational ranges)
    const float OUTER_BARO_MIN_PRESSURE = 1000;
    const float OUTER_BARO_MAX_PRESSURE = 120000;

    // Inner baro (Datasheet operational ranges)
    const float INNER_BARO_MIN_PRESSURE = 30000;
    const float INNER_BARO_MAX_PRESSURE = 110000;
    const float INNER_BARO_MIN_TEMP = -40;
    const float INNER_BARO_MAX_TEMP = 85;

    // Inner temperature probe
    const float INNER_TEMP_PROBE_MIN_TEMP = -40;
    const float INNER_TEMP_PROBE_MAX_TEMP = 60; // Safe assumption of max temp inside. If it goes higher something has gone horribly wrong

    // IMU
    // NOT SURE IF ON IMPACT THE ACC COULD BE HIGHER???
    const float IMU_MIN_ACCELERATION = -100;
    const float IMU_MAX_ACCELERATION = 100;
    // Probably won't be rotating faster that this, right???
    const float IMU_MIN_ROTATION = -36000; // 100 Hz rotations (360 deg*100)
    const float IMU_MAX_ROTATION = 36000;

    // Battery voltage
    const float BATT_VOLTAGE_MIN = 0;
    const float BATT_VOLTAGE_MAX = 8;

    // Heater current
    const float HEATER_CURRENT_MIN = 0;
    const float HEATER_CURRENT_MAX = 5;

    // Outer thermistor
    const float OUTER_THERMISTOR_MIN_TEMP = -100;
    const float OUTER_THERMISTOR_MAX_TEMP = 100;

    // detection parameters
    // Universal
    const String RESET_STATE_MSG = "reset_state";
    // Prepare
    const String ARM_MSG = "arm_confirm";
    const String DATA_SEND_MSG = "test_data_send";
    const String DATA_SEND_STOP_MSG = "test_data_stop";
    const String HEATER_ENABLE_MSG = "heater_enable";
    const String FORMAT_MSG = "format_flash";
    const String RESET_SENSOR_STATES_MSG = "reset_sensor_states";
    
    // Ascent
    const String DATA_REQUEST_MSG = "data_request";
    const String RANGING_REQUEST_MSG = "ranging_request";
    const String TOGGLE_MOSFET_1_MSG = "toggle_mosfet_one";
    const String TOGGLE_MOSFET_2_MSG = "toggle_mosfet_two";
    
    const String TELEMETRY_LOG_FILE_NAME_BASE_PATH = "/CANSAT_TELEMETRY_";
    const String INFO_LOG_FILE_NAME_BASE_PATH = "/CANSAT_INFO_";
    const String ERROR_LOG_FILE_NAME_BASE_PATH = "/CANSAT_ERROR_";

    const String TELEMETRY_HEADER = "gps_epoch_time,gps_lat,gps_lng,gps_height,gps_speed,gps_time_since_last,acc_x,acc_y,acc_z,gyro_x,gyro_y,gyro_z,r1_dist,r2_dist,r3_dist,r1_time_since,r2_time_since,r3_time_since,r_pos_lat,r_pos_lng,r_pos_alt,r_pos_time_since,inner_baro_pressure,outer_baro_pressure,avg_inner_temp,avg_outer_temp,heater_power,time_on,avg_battery_voltage,gps_heading,gps_pdop,gps_satellites,raw_inner_temp,raw_outer_temp_thermistor,raw_inner_temp_baro,raw_outer_temp_baro,raw_batt_voltage,raw_heater_current,avg_heater_current,p_term,i_term,d_term,target_temp,r1_time,r1_rssi,r1_snr,r1_f_error,r2_time,r2_rssi,r2_snr,r2_f_error,r3_time,r3_rssi,r3_snr,r3_f_error,last_frequency";
    const String INFO_HEADER = "time_on,info";
    const String ERROR_HEADER = "time_on,error";
};