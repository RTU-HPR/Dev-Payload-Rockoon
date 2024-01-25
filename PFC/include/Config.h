#pragma once
// Main libraries
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <LittleFS.h>
#include <SDFS.h>
#include <cppQueue.h>

// Public sensor libraries
#include <PCF8575.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM6DSL.h>
#include <NTC_Thermistor.h>
#include <Adafruit_BMP085.h>
#include "ClosedCube_STS35.h"
#include "ClosedCube_I2C.h"

// Our wrappers
#include <RadioLib_wrapper.h>
#include <ranging_wrapper.h>
#include <Gps_Wrapper.h>
#include <Sd_card_wrapper.h>

// Our functions
#include <Ccsds_packets.h>

// Our sensor libaries
#include <MS56XX.h>
#include <Adc_Voltage.h>

// Used radio module
#define radio_module SX1268

class Config
{
public:
    bool WAIT_PC = false;
    const bool LOG_TO_STORAGE = true;

    // 433 MHz LoRa
    RadioLib_Wrapper<radio_module>::Radio_Config radio_config{
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
        .tx_power = 10,
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
            {.position = {0, 0, 0}, .address = 0x9A8B7C6D},
    };
    Ranging_Wrapper::Mode ranging_mode = Ranging_Wrapper::Mode::MASTER;
    const int RANGING_LORA_TIMEOUT = 200;
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
            .SPI = &SPI,
    };

    // GPS
    Gps_Wrapper::Gps_Config_I2C gps_config{
        .config = {
            .timeout = 5000,                       // Time it takes for anything to timeout
            .measurement_rate = 500,               // how often measurement will be taken in ms
            .navigation_frequency = 2,             // how often tu updated navigation in s
            .dynamic_model = DYN_MODEL_AIRBORNE2g, // DYN_MODEL_AIRBORNE2g
            .com_settings = COM_TYPE_UBX,          // COM_TYPE_UBX
            .auto_pvt = true                       // for neo6m dont use this
        },
        .wire = &Wire,
        .i2c_address = 0x42, // Default
    };

    // SD card
    String TELMETRY_FILE_HEADER = "index,time,gps_lat,gps_lng,gps_altitude,outside_temperature,gps_satellites,pressure,gps_speed,baro_altitude,acc_x,acc_y,acc_z,acc_heading,acc_pitch,acc_roll,gyro_x,gyro_y,gyro_z,imu_temp,onboard_temp,container_temp,container_temp_baro,container_pressure,battery_voltage,gps_epoch,gps_heading,gps_pdop,r1_distance,r1_ferror,r1_rssi,r1_snr,r1_time,r2_distance,r2_ferror,r2_rssi,r2_snr,r2_time,r3_distance,r3_ferror,r3_rssi,r3_snr,r3_time,ranging_lat,ranging_lng,ranging_altitude,time_on_ms,used_heap,loop_time";
    String INFO_FILE_HEADER = "time,info";
    String ERROR_FILE_HEADER = "time,error";
    String CONFIG_FILE_HEADER = "descent_flag,remaining_descent_time,parachutes_deployed_flag,heater_control_flag";

    struct Config_File_Values
    {
        int descent_flag;
        long remaining_descent_time;
        int parachutes_deployed_flag;
        int heater_control_flag;
    };

    Config_File_Values config_file_values = {
        .descent_flag = 0,
        .remaining_descent_time = -1,
        .parachutes_deployed_flag = 0,
        .heater_control_flag = 0,
    };

    SD_Card_Wrapper::Config sd_card_config = {
        // spi bus
        .spi_bus = &SPI,
        .cs_pin = 9,
        .data_file_path_base = "/PFC_TELEMETRY_",
        .info_file_path_base = "/PFC_INFO_",
        .error_file_path_base = "/PFC_ERROR_",
        .config_file_path = "/PFC_CONFIG",
        .data_file_header = TELMETRY_FILE_HEADER,
        .info_file_header = INFO_FILE_HEADER,
        .error_file_header = ERROR_FILE_HEADER,
        .config_file_header = CONFIG_FILE_HEADER,
    };

    // PCF8575 Port extender
    struct PCF8575_Config
    {
        TwoWire *wire;
        int i2c_address;
    };

    PCF8575_Config pcf_config = {
        .wire = &Wire1,
        .i2c_address = 0x20,
    };

    // MS56XX
    MS56XX::MS56XX_Config ms56xx_config = {
        .wire = &Wire1,
        .i2c_address = MS56XX::MS56XX_I2C_ADDRESS::I2C_0x76, // or 0x77
        .ms56xx_type = MS56XX::MS56XX_TYPE::MS5611,          // or MS5607
        .oversampling = MS56XX::MS56XX_OVERSAMPLING::OSR_STANDARD,
    };

    // IMU
    struct IMU_Config
    {
        TwoWire *wire;
        int i2c_address;
    };

    IMU_Config imu_config = {
        .wire = &Wire1,
        .i2c_address = 0x6B, // or 0x6A
    };

    // Thermistor
    struct Thermistor_Config
    {
        int pin;
        float reference_resistor;
        float nominal_resistor;
        float nominal_temperature;
        float b_coefficient;
        int adc_resolution;
    };

    Thermistor_Config outside_thermistor_config = {
        .pin = 28,
        .reference_resistor = 10000,
        .nominal_resistor = 10000,
        .nominal_temperature = 25,
        .b_coefficient = -4050,
        .adc_resolution = 4095, // 12 bit
    };

    // Battery voltage reader
    AdcVoltage::AdcVoltage_Config battery_voltage_reader_config = {
        .pin = 26,                // Taken from the schematic
        .adc_resolution = 4095,   // 12 bit
        .reference_voltage = 3.3, // MCU voltage
        .R1_value = 51000,        // Taken from the schematic
        .R2_value = 24000,        // Taken from the schematic
    };

    // Container barometer
    struct BMP180_Config
    {
        TwoWire *wire;
        int i2c_address;
    };

    BMP180_Config BMP180_config = {
        .wire = &Wire1,
        .i2c_address = 0x77, // or 0x76
    };

    // Container temperature sensor
    struct STS35_Config
    {
        TwoWire *wire;
        int i2c_address;
    };

    STS35_Config STS35_config = {
        .wire = &Wire1,
        .i2c_address = 0x4B, // or 0x4A
    };

    // Info/error queue
    #define	QUEUE_IMPLEMENTATION	FIFO
    #define INFO_ERROR_QUEUE_SIZE 20
    #define INFO_ERROR_MAX_LENGTH 100
    
    // Watchdog
    const int WATCHDOG_TIMER = 8000; // Max is 8400

    // Sensor power
    const int SENSOR_POWER_ENABLE_PIN = 17;

    const int LAUNCH_RAIL_SWITCH_PIN = 21; // Switch

    // Wire0
    const int WIRE0_SCL = 1;
    const int WIRE0_SDA = 0;

    // Wire1
    const int WIRE1_SCL = 15;
    const int WIRE1_SDA = 14;

    // SPI0
    const int SPI0_RX = 4;
    const int SPI0_TX = 7;
    const int SPI0_SCK = 6;

    // Port extender
    const int PORT_EXTENDER_BUZZER_PIN = 1; // Buzzer
    const int PORT_EXTENDER_LED_2_PIN = 2;  // Status LED 2
    const int PORT_EXTENDER_LED_1_PIN = 3;  // Status LED

    // logging
    const int PC_BAUDRATE = 115200;

    // HEATER
    // Heater current
    const float HEATER_CURR_SENS_PIN = 27;
    const float HEATER_CURR_CONVERSION_FACTOR = 3.3 * 3.1251;
    const float HEATER_RESISTOR_VALUE = 1.1;

    // Heater PID config
    struct Heater_Config
    {
      int heater_pin;
      float Kp;
      float Ki;
      float Kd;
      float Kp_limit;
      float Ki_limit;
      float Kd_limit;
      int pwm_min;
      int pwm_max;
      float target_temp;
    };
    Heater_Config heater_config = {
      .heater_pin = 16,
      .Kp = 10,
      .Ki = 0.000025,
      .Kd = 10000,
      .Kp_limit = 1000,
      .Ki_limit = 1000,
      .Kd_limit = 500,
      .pwm_min = 0,
      .pwm_max = 1000,
      .target_temp = 35,
    };

    // Parachute
    const int PYRO_CHANNEL_1 = 19;
    const int PYRO_CHANNEL_2 = 18;
    const int PYRO_CHANNEL_FIRE_TIME = 5000;

    // Actions
    const int TIMED_ACTION_INITIAL_DELAY = 10000;
    // Data send action interval
    const int COMMUNICATION_CYCLE_INTERVAL = 12000;
    const int COMMUNICATION_RESPONSE_SEND_TIME = 5000;
    const int COMMUNICATION_ESSENTIAL_DATA_SEND_TIME = 9000;

    // Sendable commands
    const int PFC_ESSENTIAL_DATA_RESPONSE = 100;
    const int PFC_INFO_ERROR_RESPONSE = 102;
    const int PFC_COMPLETE_DATA_RESPONSE = 101;
    const int PFC_FORMAT_RESPONSE = 103;
    const int PFC_HEATER_RESPONSE = 104;
    const int PFC_PYRO_RESPONSE = 105;

    // Receiveable commands
    const int PFC_INFO_ERROR_REQUEST = 1001;
    const int PFC_COMPLETE_DATA_REQUEST = 1000;
    const int PFC_FORMAT_REQUEST = 1002;
    const int PFC_HEATER_REQUEST = 1003;
    const int PFC_PYRO_REQUEST = 1004;
};