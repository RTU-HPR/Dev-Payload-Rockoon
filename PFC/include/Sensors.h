#pragma once
#include <Config.h>
#include <Logging.h>

class Sensors
{
private:
  /**
   * @brief Object representing the MS56XX sensor.
   *
   * This class provides functionality to interface with the MS56XX sensor.
   */
  MS56XX _onBoardBaro;

  /**
   * @brief Object representing the IMU.
   *
   * This class provides functionality to interface with the IMU.
   */
  Adafruit_LSM6DS _imu;

  /**
   * @brief Object representing the thermistor.
   *
   * This class provides functionality to interface with the thermistor.
   */
  NTC_Thermistor _outsideThermistor = NTC_Thermistor(0, 0, 0, 0, 0);

  /**
   * @brief Object representing the battery voltage reader.
   *
   * This class provides functionality to interface with the battery voltage reader.
   */
  AdcVoltage _batteryVoltageReader;

  /**
   * @brief Object representing the container barometer.
   *
   * This class provides functionality to interface with the BMP180 barometer.
   */
  Adafruit_BMP085 _containerBaro;

  /**
   * @brief Object representing the container temperature sensor.
   *
   * This class provides functionality to interface with the STS35 temperature sensor.
   */
  ClosedCube::Sensor::STS35 _containerTemperatureSensor;

  /**
   * @brief Structure to hold IMU sensor data.
   */
  struct IMU_Data
  {
    sensors_event_t accel; // Accelerometer sensor event data.
    sensors_event_t gyro;  // Gyroscope sensor event data.
    sensors_event_t temp;  // Temperature sensor event data.
  };

  /**
   * @brief Structure to hold thermistor data.
   */
  struct Thermistor_Data
  {
    float temperature;
  };

  /**
   * @brief Structure to hold container barometer data.
   */
  struct ContainerBarometer_Data
  {
    float temperature;
    float pressure;
  };

  /**
   * @brief Structure to hold container temperature data.
   */
  struct ContainerTemperature_Data
  {
    float temperature;
  };

public:
  String sensorErrorString = "";
  
  /**
   * @brief Structure to store all sensor data
   */
  struct SENSOR_DATA
  {
    MS56XX::MS56XX_Data onBoardBaro;
    IMU_Data imu;
    AdcVoltage::AdcVoltage_Data battery;
    Thermistor_Data outsideThermistor;
    ContainerBarometer_Data containerBaro;
    ContainerTemperature_Data containerTemperature;
  };

  SENSOR_DATA data;

  /**
   * @brief Initializes all sensor objects and gets them ready to be used.
   *
   * @param config The configuration object containing the sensor settings.
   * @return True if the initialization is successful, false otherwise.
   */
  bool begin(Logging &logging, Config &config);

  /**
   * @brief Reads data from all sensors.
   *
   */
  void readSensors();

  /**
   * Initializes the port extender.
   *
   * @param config The configuration object.
   * @return True if the port extender was successfully initialized, false otherwise.
   */
  bool beginPortExtender(Config &config);

  /**
   * Initializes the on-board barometer sensor.
   *
   * @param config The configuration object.
   * @return True if the sensor initialization is successful, false otherwise.
   */
  bool beginOnBoardBaro(Config &config);

  /**
   * Initializes the IMU sensor.
   *
   * @param config The configuration object.
   * @return True if the initialization is successful, false otherwise.
   */
  bool beginImu(Config &config);

  /**
   * @brief Initializes the outside thermistor sensor.
   *
   * @param config The configuration object for the sensor.
   * @return True if the initialization is successful, false otherwise.
   */
  bool beginOutsideThermistor(Config &config);

  /**
   * @brief Initializes the battery voltage reader.
   *
   * @param config The configuration object containing the necessary settings for the battery voltage reader.
   * @return True if the initialization is successful, false otherwise.
   */
  bool beginBatteryVoltageReader(Config &config);

  /**
   * @brief Initializes the container barometer sensor.
   *
   * @param config The configuration object containing the settings for the sensor.
   * @return True if the initialization was successful, false otherwise.
   */
  bool beginContainerBaro(Config &config);

  /**
   * Initializes the container temperature sensor.
   *
   * @param config The configuration object for the sensor.
   * @return True if the sensor was successfully initialized, false otherwise.
   */
  bool beginContainerTemperatureSensor(Config &config);

  /**
   * @brief Reads data from the onboard barometer.
   *
   * @return true if the data was successfully read, false otherwise.
   */
  bool readOnBoardBaro();

  /**
   * @brief Reads data from the IMU.
   *
   * @return true if the data was successfully read, false otherwise.
   */
  bool readImu();

  /**
   * @brief Reads the battery voltage.
   *
   * @return true if the battery voltage was successfully read, false otherwise.
   */
  bool readBatteryVoltage();

  /**
   * @brief Reads the outside thermistor temperature.
   *
   * @return true if the temperature was successfully read, false otherwise.
   */
  bool readOutsideThermistor();

  /**
   * @brief Reads the container barometer temperature and pressure.
   *
   * @return true if the temperature and pressure were successfully read, false otherwise.
   */
  bool readContainerBarometer();

  /**
   * @brief Reads the container temperature sensor.
   *
   * @return true if the temperature was successfully read, false otherwise.
   */
  bool readContainerTemperature();
};