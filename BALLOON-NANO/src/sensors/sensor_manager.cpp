#include "sensors/sensor_manager.h"

// PRIVATE FUNCTIONS
void Sensor_manager::read_batt_voltage(Log &log, Config &config)
{
    // Read voltage and do calculations
    float adc_reading = analogRead(config.BATT_SENS_PIN);
    float new_batt_voltage = (adc_reading / 4095.0) * config.BATT_SENS_CONVERSION_FACTOR;

    // Just make sure the voltage value is within reasonable values
    if (config.BATT_VOLTAGE_MIN < new_batt_voltage && new_batt_voltage < config.BATT_VOLTAGE_MAX)
    {
        data.batt_voltage = new_batt_voltage;
        _batt_averager->add_data(data.batt_voltage);
        data.average_batt_voltage = _batt_averager->get_averaged_value();
    }
    else
    {
        log.send_info("Battery voltage reading outside range: " + String(new_batt_voltage));
    }
    // NO OTHER CHECKS ARE DONE AS THIS IS VERY SIMPLE AND NOTHING CAN REALLY GO WRONG (Hopefully)
}

void Sensor_manager::read_heater_current(Log &log, Config &config)
{
    // Formula is from linear regression from experimental data. See #payload channel in Discord on 3/12/2023
    if ((int)_temp_manager->get_heater_power() == 0)
    {
        data.heater_current = 0;    
    }
    else
    {
        data.heater_current = 0.002331 * (int)_temp_manager->get_heater_power() + 0.034313;
    }
    _heater_current_averager->add_data(data.heater_current);
    data.average_heater_current = _heater_current_averager->get_averaged_value();
}

void Sensor_manager::position_calculation(Log &log, Config &config)
{
    // DON'T KNOW WHAT ARE THE EXPECTED VALUES SO ERROR CHECKING WILL BE IMPLEMENTED LATER
    if (_ranging_lora_initalized)
    {
        Ranging_Wrapper::Position result;
        if (_ranging_lora.trilaterate_position(data.ranging_results, config.RANGING_SLAVES, result))
        {
            data.ranging_position = result;
            _last_ranging_pos_time = millis();
        }
    }
    // maybe do more processing
    return;
}

void Sensor_manager::read_ranging(Config &config)
{
    // DON'T KNOW WHAT ARE THE EXPECTED VALUES SO ERROR CHECKING WILL BE IMPLEMENTED LATER
    // try to range next slave address
    if (_ranging_lora_initalized)
    {
        Ranging_Wrapper::Ranging_Result result = {0, 0};
        bool move_to_next_slave = false;
        if (_ranging_lora.master_read(config.RANGING_SLAVES[_slave_index], result, config.RANGING_LORA_TIMEOUT))
        {
            // ranging data read and ranging for current slave started
            move_to_next_slave = true;
        }
        // check if something useful was read from the previous slave
        if (result.distance != 0 && result.time != 0)
        {
            data.ranging_results[_last_slave_index] = result;
        }

        // move to next slave
        if (move_to_next_slave)
        {
            _last_slave_index = _slave_index;
            int array_length = 3;
            _slave_index++;
            if (_slave_index > array_length - 1)
            {
                // reset index
                _slave_index = 0;
            }
        }
    }
}

void Sensor_manager::read_gps(Log &log)
{
    if (!_gps_initialized)
    {
        return;
    }

    if (_gps.getPVT() && (_gps.getInvalidLlh() == false))
    {
        long new_gps_lat_raw = _gps.getLatitude();
        long new_gps_lng_raw = _gps.getLongitude();

        double new_gps_lat = new_gps_lat_raw / 10000000.0;
        double new_gps_lng = new_gps_lng_raw / 10000000.0;

        // SANITY CHECK
        // Check if location is 0 (not yet established) or somewhere in the northern eastern Europe
        if ((new_gps_lat == 0 && new_gps_lng == 0) || ((50 <= new_gps_lat && new_gps_lat <= 60) && (15 <= new_gps_lng && new_gps_lng <= 35)))
        {
            _last_gps_packet_time = millis();
            data.gps_lat = new_gps_lat;
            data.gps_lng = new_gps_lng;
            data.gps_height = _gps.getAltitude() / 1000.0;
            data.gps_satellites = _gps.getSIV();
            data.gps_speed = _gps.getGroundSpeed() / 1000.0;
            data.gps_heading = _gps.getHeading() / 10000.0;
            data.gps_pdop = _gps.getPDOP() / 100.0;
            data.gps_epoch_time = _gps.getUnixEpoch();
        }
        else
        {
            Serial.println("GPS location is not correct: " + String(new_gps_lat, 6) + " " + String(new_gps_lng, 6));
        }
    }
}

void Sensor_manager::read_outer_baro(Log &log, Config &config)
{
    // Check if initalized or if the sensor has been flagged as failed
    if (!_outer_baro_initialized || config.last_state_variables.outer_baro_failed)
    {
        return;
    }
    else
    {
        // Start timer
        unsigned long int reading_start = millis();
        unsigned long int reading_end;

        bool data_ok = true;

        // Try to read the sensor
        _outer_baro.read();

        float new_pressure = _outer_baro.getPressure() * 100;
        float new_temperature = _outer_baro.getTemperature();
        // Check if values within acceptable range
        if (config.OUTER_BARO_MIN_PRESSURE <= new_pressure && new_pressure <= config.OUTER_BARO_MAX_PRESSURE)
        {
            data.outer_baro_pressure = new_pressure;
            // Assume that temp reading is also good
            data.outer_baro_temp = new_temperature;
            // Hypsometric formula
            float new_altitude = ((pow((config.SEA_LEVEL_PRESSURE/new_pressure), (1/5.257)) - 1) * (new_temperature + 273.15)) / 0.065;
            data.outer_baro_altitude = new_altitude;
            data.outer_baro_altitude_speed = (new_altitude - data.outer_baro_altitude) / (float)(millis() - _last_baro_reading_time);
        }
        else
        {
            // Later change to log_error_to_flash
            log.send_info("Outer baro pressure reading outside range: " + String(new_pressure));
            data_ok = false;
        }
        // TEMPERATURE IS NOT CHECKED AS OUTSIDE TEMP CAN BE OUTSIDE SENSOR RANGE
        // AND THERE IS A CHANCE THAT TEMP CAN BE SIMPLY INACCURATE
        // End timer, as the actual reading/processing part has ended
        reading_end = millis();

        // Log if sensor reading failed
        if (!data_ok)
        {
            _outer_baro_consecutive_failed_readings += 1;
            log.send_info("Reading outer baro failed. Consecutive attempt: " + String(_outer_baro_consecutive_failed_readings));
            if (_outer_baro_consecutive_failed_readings >= config.OUTER_BARO_MAX_ATTEMPTS)
            {
                // Set sensor to failed
                log.send_info("Outer baro failure detected!");
                config.last_state_variables.outer_baro_failed = 1;
                _outer_baro_initialized = false;
            }
        }
        // If the actual reading of the sensor took too long, something is probably wrong with it
        if (reading_end - reading_start >= config.OUTER_BARO_TIMEOUT)
        {
            // Set sensor to failed
            //log.send_info("Outer baro timeout detected!");
            //config.last_state_variables.outer_baro_failed = 1;
            //_outer_baro_initialized = false;
        }
    }
}

void Sensor_manager::read_inner_baro(Log &log, Config &config)
{
    // Check if initalized or if the sensor has been flagged as failed
    if (!_inner_baro_initialized || config.last_state_variables.inner_baro_failed)
    {
        return;
    }
    else
    {
        // Start timer
        unsigned long int reading_start = millis();
        unsigned long int reading_end;

        bool data_ok = true;

        // Try to read the sensor
        float new_pressure = _inner_baro.readPressure();
        float new_temperature = _inner_baro.readTemperature();

        // Check if values within acceptable range
        // Pressure
        if (config.INNER_BARO_MIN_PRESSURE <= new_pressure && new_pressure <= config.INNER_BARO_MAX_PRESSURE)
        {
            data.inner_baro_pressure = new_pressure;
        }
        else
        {
            // Later change to log_error_to_flash
            log.send_info("Inner baro pressure reading outside range: " + String(new_pressure));
            data_ok = false;
        }

        // Temperature
        if (config.INNER_BARO_MIN_TEMP <= new_temperature && new_temperature <= config.INNER_BARO_MAX_TEMP)
        {
            data.inner_baro_temp = new_temperature;
            _inner_temp_averager_baro->add_data(data.inner_baro_temp);
            data.average_inner_temp_baro = _inner_temp_averager_baro->get_averaged_value();
        }
        else
        {
            // Later change to log_error_to_flash
            log.send_info("Inner baro temperature reading outside range: " + String(new_temperature));
            data_ok = false;
        }

        // End timer, as the actual reading/processing part has ended
        reading_end = millis();

        // Log if sensor reading failed
        if (!data_ok)
        {
            _inner_baro_consecutive_failed_readings += 1;
            log.send_info("Reading inner baro failed. Consecutive attempt: " + String(_outer_baro_consecutive_failed_readings));
            if (_inner_baro_consecutive_failed_readings >= config.INNER_BARO_MAX_ATTEMPTS)
            {
                // Set sensor to failed
                log.send_info("Inner baro failure detected!");
                config.last_state_variables.inner_baro_failed = 1;
                _inner_baro_initialized = false;
            }
        }
        // If the actual reading of the sensor took too long, something is probably wrong with it
        if (reading_end - reading_start >= config.INNER_BARO_TIMEOUT)
        {
            // Set sensor to failed
            //log.send_info("Inner baro timeout detected!");
            //_inner_baro_initialized = false;
        }
    }
}

void Sensor_manager::read_time()
{
    data.time = millis();
    data.time_since_last_gps = data.time - _last_gps_packet_time;
    data.time_since_last_ranging_pos = data.time - _last_ranging_pos_time;
    for (int i = 0; i <= 2;)
    {
        data.times_since_last_ranging_result[i] = data.time - data.ranging_results[i].time;
        i++;
    }
}

void Sensor_manager::read_imu(Log &log, Config &config)
{
    // Check if initalized or if the sensor has been flagged as failed
    if (!_imu_initialized || config.last_state_variables.imu_failed)
    {
        return;
    }
    else
    {
        // Start timer
        unsigned long int reading_start = millis();
        unsigned long int reading_end;

        bool data_ok = true;

        // Read the sensor
        sensors_event_t accel;
        sensors_event_t gyro;
        sensors_event_t temp;
        _imu.getEvent(&accel, &gyro, &temp);

        // End timer, as the actual reading part has ended
        reading_end = millis();

        // Get largest and smallest data values
        double acc_min = min(min(accel.acceleration.x, accel.acceleration.y), accel.acceleration.z);
        double acc_max = max(max(accel.acceleration.x, accel.acceleration.y), accel.acceleration.z);
        double gyro_min = min(min(gyro.gyro.x, gyro.gyro.y), gyro.gyro.z);
        double gyro_max = max(max(gyro.gyro.x, gyro.gyro.y), gyro.gyro.z);

        // Check if values within acceptable range
        // Acceleration
        // Check if all values are 0, as that is not possible due to noise
        // That means that sensor reading has failed
        if (accel.acceleration.x == 0 && accel.acceleration.y == 0 && accel.acceleration.z == 0)
        {
            // Later change to log_error_to_flash
            log.send_info("IMU acceleration readings are 0");
            data_ok = false;
        }
        else if (config.IMU_MIN_ACCELERATION <= acc_min && acc_max <= config.IMU_MAX_ACCELERATION)
        {
            data.acc[0] = accel.acceleration.x;
            data.acc[1] = accel.acceleration.y;
            data.acc[2] = accel.acceleration.z;
        }
        else
        {
            // Later change to log_error_to_flash
            log.send_info("IMU acceleration readings outside range: " + String(accel.acceleration.x) + " " + String(accel.acceleration.y) + " " + String(accel.acceleration.z));
            data_ok = false;
        }

        // Gyro
        // Check if all values are 0, as that is not possible due to noise
        // That means that sensor reading has failed
        if (gyro.gyro.x == 0 && gyro.gyro.y == 0 && gyro.gyro.z == 0)
        {
            // Later change to log_error_to_flash
            log.send_info("IMU gyro readings are 0");
            data_ok = false;
        }
        else if (config.IMU_MIN_ROTATION <= gyro_min && gyro_max <= config.IMU_MAX_ROTATION)
        {
            data.gyro[0] = gyro.gyro.x;
            data.gyro[1] = gyro.gyro.y;
            data.gyro[2] = gyro.gyro.z;
        }
        else
        {
            // Later change to log_error_to_flash
            log.send_info("IMU gyro readings outside range: " + String(gyro.gyro.x) + " " + String(gyro.gyro.y) + " " + String(gyro.gyro.z));
            data_ok = false;
        }

        // Log if sensor reading failed
        if (!data_ok)
        {
            _imu_consecutive_failed_readings += 1;
            log.send_info("Reading IMU failed. Consecutive attempt: " + String(_imu_consecutive_failed_readings));
            if (_imu_consecutive_failed_readings >= config.IMU_MAX_ATTEMPTS)
            {
                log.send_info("IMU failure detected!");
                _imu_initialized = false;
            }
        }
        // If the actual reading of the sensor took too long, something is probably wrong with it
        if (reading_end - reading_start >= config.IMU_TIMEOUT)
        {
            //log.send_info("IMU timeout detected!");
            //_imu_initialized = false;
        }

        // Restart Pico if sensor has failed and if it hasn't already been restarted
        if (!_imu_initialized && !config.last_state_variables.imu_restarted)
        {
            _hard_reset_required = true;
        }
    }
}

void Sensor_manager::read_inner_temp_probe(Log &log, Config &config)
{
    if (!_inner_temp_probe_initialized || config.last_state_variables.inner_temp_probe_failed)
    {
        return;
    }
    else
    {
        // Start timer
        unsigned long int reading_start = millis();
        unsigned long int reading_end;

        bool data_ok = true;

        // Read the sensor
        float new_temperature = _inner_temp_probe.readTemperature();

        // End timer, as the actual reading part has ended
        reading_end = millis();

        // Check if value within acceptable range
        if (config.INNER_TEMP_PROBE_MIN_TEMP <= new_temperature && new_temperature <= config.INNER_TEMP_PROBE_MAX_TEMP)
        {
            data.inner_temp_probe = new_temperature;
            _inner_temp_averager->add_data(data.inner_temp_probe);
            data.average_inner_temp = _inner_temp_averager->get_averaged_value();
        }
        else
        {
            // Later change to log_error_to_flash
            log.send_info("Inner probe temp reading outside range: " + String(new_temperature));
            data_ok = false;
        }

        // Log if sensor reading failed
        if (!data_ok)
        {
            _inner_temp_probe_consecutive_failed_readings += 1;
            log.send_info("Reading inner probe failed. Consecutive attempt: " + String(_inner_temp_probe_consecutive_failed_readings));
            if (_inner_temp_probe_consecutive_failed_readings >= config.INNER_TEMP_PROBE_MAX_ATTEMPTS)
            {
                log.send_info("Inner temp probe failure detected!");
                _inner_temp_probe_initialized = false;
            }
        }

        // If the actual reading of the sensor took too long, something is probably wrong with it
        if (reading_end - reading_start >= config.INNER_TEMP_PROBE_TIMEOUT)
        {
            log.send_info("Inner temp probe timeout detected!");
            _inner_temp_probe_initialized = false;
        }

        // Restart Pico if sensor has failed and if it hasn't already been restarted
        if (!_inner_temp_probe_initialized && !config.last_state_variables.inner_temp_probe_restarted)
        {
            _hard_reset_required = true;
        }
    }
}

void Sensor_manager::read_outer_thermistor(Log &log, Config &config)
{
    if (!_outer_thermistor_initialized || config.last_state_variables.outer_thermistor_failed)
    {
        return;
    }
    else
    {
        // Start timer
        unsigned long int reading_start = millis();
        unsigned long int reading_end;

        bool data_ok = true;

        // Read the sensor
        float new_temperature = _outer_thermistor.readCelsius();

        // End timer, as the actual reading part has ended
        reading_end = millis();

        // Check if value within acceptable range
        if (config.OUTER_THERMISTOR_MIN_TEMP && new_temperature <= config.OUTER_THERMISTOR_MAX_TEMP)
        {
            data.outer_temp_thermistor = new_temperature;
            _outer_temp_averager->add_data(data.outer_temp_thermistor);
            data.average_outer_temp = _outer_temp_averager->get_averaged_value();
        }
        else
        {
            // Later change to log_error_to_flash
            log.send_info("Outer thermistor reading outside range: " + String(new_temperature));
            data_ok = false;
        }

        // Log if sensor reading failed
        if (!data_ok)
        {
            _outer_thermistor_consecutive_failed_readings += 1;
            log.send_info("Reading outer thermistor failed. Consecutive attempt: " + String(_outer_thermistor_consecutive_failed_readings));
            if (_outer_thermistor_consecutive_failed_readings >= config.INNER_TEMP_PROBE_MAX_ATTEMPTS)
            {
                // Set sensor to failed
                log.send_info("Outer thermistor failure detected!");
                config.last_state_variables.outer_thermistor_failed = 1;
                _outer_thermistor_initialized = false;
            }
        }

        // If the actual reading of the sensor took too long, something is probably wrong with it
        if (reading_end - reading_start >= config.OUTER_BARO_TIMEOUT)
        {
            // Set sensor to failed
            log.send_info("Outer thermistor timeout detected!");
            config.last_state_variables.outer_thermistor_failed = 1;
            _outer_thermistor_initialized = false;
        }
    }
}

void Sensor_manager::update_heater(Log &log, Config &config)
{
    if (_heater_enabled)
    {
        float best_inner_temp;

        // If heater is not set to constant power mode (both temp sensors have failed)
        if (!_heater_constant)
        {
            // Compare inner temp probe and inner baro temp
            // If both sensors working
            if (_inner_temp_probe_initialized && _inner_baro_initialized)
            {
                // If the temp difference between both sensors is less 5 degrees
                // both sensors are probably working fine
                if (abs(data.average_inner_temp - data.average_inner_temp_baro) <= 10)
                {
                    best_inner_temp = data.average_inner_temp;
                }
                // If the difference is larger, take the smallest value to be on the safe side of things
                else
                {
                    best_inner_temp = min(data.average_inner_temp, data.average_inner_temp_baro);
                    log.send_info("Difference between inner temp sensors larger than 5: " + String(data.average_inner_temp) + " " + String(data.average_inner_temp_baro));
                }
            }
            // If only inner temp probe is working
            else if (_inner_temp_probe_initialized)
            {
                best_inner_temp = data.average_inner_temp;
            }
            // From testing inner baro temp was always lower than inner temp probe, so taking this value should be fine
            else if (_inner_baro_initialized)
            {
                best_inner_temp = data.average_inner_temp_baro;
            }
            // If both sensors have failed there are 2 options:
            // * We set inner temp to target temp, which sets proportional term to 0, and only leaves integral term
            //   Assuming the inner box has reached equilibrium, heating power will stay constant and temperature should also
            //   But as the heating losses will not be constant, temperature can increase way above target temp
            //   If we slightly reduce the integral term, the temperature should in most cases stay below the target temp
            //
            // * Other option is to just disable the heater. This means that temperature can't ever go higher than target temp
            //   But also this means that inner temp can go way below safe temp, especially while we are still ascending
            // Probably the first option is better, as the chance that the experiment is still somewhat successful is higher
            else
            {
                _heater_constant_temp = _temp_manager->_safe_temp;
                // Reduce the integral term by 30%
                _temp_manager->_integral_term *= 0.7;
                _heater_constant = true;
            }
        }
        // If heater is set to constant power, set it to constant heater temp, which makes proportional term equal to 0
        if (_heater_constant)
        {
            best_inner_temp = _heater_constant_temp;
        }

        // Send the best option of inner temp to temp control
        _temp_manager->update_heater_power(best_inner_temp);

        data.heater_power = _temp_manager->get_heater_power();
        _temp_manager->get_pid(data.p, data.i, data.d);
        data.target_temp = _temp_manager->get_target_temp();

        // Check if heater should be disabled, because of critical battery voltage
        if (data.average_batt_voltage < config.HEATER_CUT_OFF_VOLTAGE)
        {
            _heater_enabled = false;
            set_heater(false);
        }
    }
}

// PUBLIC FUNCTIONS
void Sensor_manager::init(Log &log, Config &config)
{
    Wire.setSCL(config.WIRE0_SCL);
    Wire.setSDA(config.WIRE0_SDA);
    Wire.begin();

    // GPS
    //_gps.enableDebugging();
    Serial.print("Starting GPS ");
    while (_gps.begin(Wire, 0x42) == false)
    {
        Serial.print(".");
    }
    Serial.println(" done");
    _gps.setI2COutput(COM_TYPE_UBX); // Set the I2C port to output UBX only (turn off NMEA noise)
    _gps.setMeasurementRate(500);    // 2 Hz
    _gps.setNavigationFrequency(2);  // Produce two solutions per second
    _gps.setAutoPVT(true);           // Tell the GNSS to "send" each solution

    if (!_gps.setDynamicModel(DYN_MODEL_AIRBORNE2g)) // Set the dynamic model to Airborne2g
    {
        log.send_info("GPS Dynamic model setting error");
        // status += "GPS dynamic model error ";
    }
    else
    {
        _gps_initialized = true;
    }
    // Change analogRead resolution
    analogReadResolution(12);

    Wire1.setSCL(config.WIRE1_SCL);
    Wire1.setSDA(config.WIRE1_SDA);
    Wire1.begin();

    // Port extender
    PCF8575 _port_extender = PCF8575(0x20, &Wire1);
    _port_extender.begin();
    _port_extender.write(config.PORT_EXTENDER_BUZZER_PIN, LOW);
    _port_extender.write(config.PORT_EXTENDER_LED_2_PIN, LOW);
    _port_extender.write(config.PORT_EXTENDER_LED_1_PIN, LOW);    

    // Outer baro
    if (!config.last_state_variables.outer_baro_failed)
    {
        _outer_baro = MS5611(0x76);
        if (!_outer_baro.begin(&Wire1))
        {
            log.send_error("MS5611 init error");
        }
        else
        {
            _outer_baro.setOversampling(OSR_LOW); // OSR_ULTRA_LOW => 0.5 ms/OSR_LOW => 1.1 ms
            _outer_baro_initialized = true;
        }
    }
    else
    {
        log.send_error("MS5611 state is set as failed. Sensor not initalized");
    }

    // Inner baro
    if (!config.last_state_variables.inner_baro_failed)
    {
        _inner_baro = Adafruit_BMP085();
        if (!_inner_baro.begin(config.BMP180_ADDRESS_I2C, &Wire1))
        {
            log.send_error("BMP180 init error");
        }
        else
        {
            _inner_baro_initialized = true;
        }
    }
    else
    {
        log.send_error("BMP280 state is set as failed. Sensor not initalized");
    }
    // IMU WIRE1
    if (!config.last_state_variables.imu_failed)
    {
        if (_imu.begin_I2C(0x6B, &Wire1))
        {
            log.send_error("IMU init error");
        }
        else
        {
            _imu.setAccelRange(LSM6DS_ACCEL_RANGE_2_G);
            _imu.setGyroRange(LSM6DS_GYRO_RANGE_1000_DPS);
            _imu.setAccelDataRate(LSM6DS_RATE_104_HZ);
            _imu.setGyroDataRate(LSM6DS_RATE_104_HZ);
            _imu_initialized = true;
        }
    }
    else
    {
        log.send_error("IMU state is set as failed. Sensor not initalized");
    }
    // TEMP PROBE
    if (!config.last_state_variables.inner_temp_probe_failed)
    {
        _inner_temp_probe = ClosedCube::Sensor::STS35(&Wire1);
        _inner_temp_probe.address(0x4B);
        _inner_temp_probe_initialized = true;

        // Test temp probe to see if its working
        float test = _inner_temp_probe.readTemperature();
        if (test > 100.00 || test < -100.0 || test == 0.00)
        {
            _inner_temp_probe_initialized = false;
            log.send_error("Inner temp probe init error");
        }
    }
    else
    {
        log.send_error("Inner temp probe state is set as failed. Sensor not initalized");
    }
    // Outer temp probe
    if (!config.last_state_variables.outer_thermistor_failed)
    {
        _outer_thermistor = NTC_Thermistor(config.THERMISTOR_PIN, config.THERMISTOR_REFERENCE_R, config.THERMISTOR_NOMINAL_R, config.THERMISTOR_NOMINAL_T, config.THERMISTOR_B, 4095);
        _outer_thermistor_initialized = true;
    }
    else
    {
        log.send_error("Outer_thermistor state is set as failed. Sensor not initalized");
    }
    // TEMP CALCULATOR
    _temp_manager = new Temperature_Manager(config.HEATER_MOSFET, config.DESIRED_HEATER_TEMP);

    // TEMP averagers
    _inner_temp_averager = new Time_Averaging_Filter<float>(config.INNER_TEMP_AVERAGE_CAPACITY, config.INNER_TEMP_AVERAGE_TIME);
    _inner_temp_averager_baro = new Time_Averaging_Filter<float>(config.INNER_TEMP_AVERAGE_CAPACITY, config.INNER_TEMP_AVERAGE_TIME);
    _outer_temp_averager = new Time_Averaging_Filter<float>(config.OUTER_TEMP_AVERAGE_CAPACITY, config.OUTER_TEMP_AVERAGE_TIME);

    // Battery
    pinMode(config.BATT_SENS_PIN, INPUT);
    _batt_averager = new Time_Averaging_Filter<float>(config.BAT_AVERAGE_CAPACITY, config.BAT_AVERAGE_TIME);

    // Heater current
    pinMode(config.HEATER_CURR_SENS_PIN, INPUT);
    _heater_current_averager = new Time_Averaging_Filter<float>(100, config.BAT_AVERAGE_TIME);
    // RANGING lora
    String result = _ranging_lora.init(config.LORA2400_MODE, config.ranging_device);
    if (result == "")
    {
        _ranging_lora_initalized = true;
    }
    else
    {
        log.send_error("Ranging LoRa init error");
    }
}

void Sensor_manager::reset_sensor_power(Config &config)
{
    unsigned long int start = millis();
    // Disable 3.3 V power bus
    digitalWrite(config.SENSOR_POWER_ENABLE_PIN, LOW);

    while (millis() - start >= 250)
    {
        delay(1);
    }
    // Enable 3.3 V power bus
    digitalWrite(config.SENSOR_POWER_ENABLE_PIN, HIGH);
}

bool Sensor_manager::read_switch_state(Config &config)
{
    bool switch_state = digitalRead(config.LAUNCH_RAIL_SWITCH_PIN);
    return switch_state;
}

void Sensor_manager::read_data(Log &log, Config &config)
{
    // Get data from all sensors
    // GPS
    read_gps(log);

    // IMU
    read_imu(log, config);

    // Baro
    read_outer_baro(log, config);
    read_inner_baro(log, config);

    // Temperature
    read_inner_temp_probe(log, config);
    read_outer_thermistor(log, config);

    // Battery voltage
    read_batt_voltage(log, config);

    // Heater current
    read_heater_current(log, config);

    // Heater (Must be after baro, temp and voltage)
    update_heater(log, config);

    // MISC.
    // read_ranging(log, config);
    position_calculation(log, config);
    read_time();
    // Serial.println("PID: " + String(data.p) + " " + String(data.i) + " " + String(data.d) + " | Safe temp: " + String(_temp_manager->_safe_temp) + " | Inner temp: " + String(data.average_inner_temp) + " | Heater pwm: " + String(_temp_manager->get_heater_power()) + " | Heater current: " + String(data.average_heater_current) + " | Heater power: " + String(data.average_heater_current * data.average_batt_voltage));
    get_data_packets(sendable_packet, loggable_packet);
}
void Sensor_manager::get_data_packets(String &sendable_packet, String &loggable_packet)
{
    update_data_packet(data, sendable_packet, loggable_packet);
}

// Updates the message data packets with newest sensor data
void Sensor_manager::update_data_packet(Sensor_data &data, String &result_sent, String &result_log)
{
    String packet;
    // GPS
    packet += String(data.gps_epoch_time, DEC); // 1
    packet += ",";
    packet += String(data.gps_lat, 6); // 2
    packet += ",";
    packet += String(data.gps_lng, 6); // 3
    packet += ",";
    packet += String(data.gps_height, 2); // 4
    packet += ",";
    packet += String(data.gps_speed, 2); // 5
    packet += ",";
    packet += String(data.time_since_last_gps); // 6
    packet += ",";

    // Accelerometer
    packet += String(data.acc[0], 4); // 7
    packet += ",";
    packet += String(data.acc[1], 4); // 8
    packet += ",";
    packet += String(data.acc[2], 4); // 9
    packet += ",";

    // Gyro
    packet += String(data.gyro[0], 2); // 10
    packet += ",";
    packet += String(data.gyro[1], 2); // 11
    packet += ",";
    packet += String(data.gyro[2], 2); // 12
    packet += ",";

    // Ranging
    packet += String(data.ranging_results[0].distance, 2); // 13
    packet += ",";
    packet += String(data.ranging_results[1].distance, 2); // 14
    packet += ",";
    packet += String(data.ranging_results[2].distance, 2); // 15
    packet += ",";
    packet += String(data.times_since_last_ranging_result[0]); // 16
    packet += ",";
    packet += String(data.times_since_last_ranging_result[1]); // 17
    packet += ",";
    packet += String(data.times_since_last_ranging_result[2]); // 18
    packet += ",";
    packet += String(data.ranging_position.lat, 6); // 19
    packet += ",";
    packet += String(data.ranging_position.lng, 6); // 20
    packet += ",";
    packet += String(data.ranging_position.height, 2); // 21
    packet += ",";
    packet += String(data.time_since_last_ranging_pos); // 22
    packet += ",";

    // Baro
    packet += String((int)data.inner_baro_pressure); // 23
    packet += ",";
    packet += String((int)data.outer_baro_pressure); // 24
    packet += ",";

    // Temperatures
    packet += String(data.average_inner_temp, 2); // 25
    packet += ",";
    packet += String(data.average_outer_temp, 2); // 26
    packet += ",";

    // Heater
    packet += String(data.heater_power, 2); // 27
    packet += ",";

    // Misc
    packet += String(data.time); // 28
    packet += ",";
    packet += String(data.average_batt_voltage, 2); // 29
    packet += ",";
    packet += String(data.outer_baro_altitude, 2);
    packet += ",";
    packet += String(data.outer_baro_altitude_speed, 2);

    result_sent = packet;

    packet += ",";

    // GPS
    packet += String(data.gps_heading, 2); // 30
    packet += ",";
    packet += String(data.gps_pdop, 2); // 31
    packet += ",";
    packet += String(data.gps_satellites); // 32
    packet += ",";

    // Temperatures
    packet += String(data.inner_temp_probe, 2); // 33
    packet += ",";
    packet += String(data.outer_temp_thermistor, 2); // 34
    packet += ",";
    packet += String(data.inner_baro_temp, 2); // 35
    packet += ",";
    packet += String(data.outer_baro_temp, 2); // 36
    packet += ",";
    
    // Voltage/current
    packet += String(data.batt_voltage, 2); // 37
    packet += ",";
    packet += String(data.heater_current, 2); // 38
    packet += ",";
    packet += String(data.average_heater_current, 2); // 39
    packet += ",";

    // PID
    packet += String(data.p, 2); // 40
    packet += ",";
    packet += String(data.i, 2); // 41
    packet += ",";
    packet += String(data.d, 2); // 42
    packet += ",";
    packet += String((int)data.target_temp); // 43
    packet += ",";

    // Ranging
    packet += String(data.ranging_results[0].time); // 44
    packet += ",";
    packet += String(data.ranging_results[0].rssi, 2); // 45
    packet += ",";
    packet += String(data.ranging_results[0].snr, 2); // 46
    packet += ",";
    packet += String(data.ranging_results[0].f_error, 2); // 47
    packet += ",";
    packet += String(data.ranging_results[1].time); // 48
    packet += ",";
    packet += String(data.ranging_results[1].rssi, 2); // 49
    packet += ",";
    packet += String(data.ranging_results[1].snr, 2); // 50
    packet += ",";
    packet += String(data.ranging_results[1].f_error, 2); // 51
    packet += ",";
    packet += String(data.ranging_results[2].time); // 52
    packet += ",";
    packet += String(data.ranging_results[2].rssi, 2); // 53
    packet += ",";
    packet += String(data.ranging_results[2].snr, 2); // 54
    packet += ",";
    packet += String(data.ranging_results[2].f_error, 2); // 55

    packet += ",";
    packet += String(data.last_frequency, 8);
    
    result_log = packet;
}
