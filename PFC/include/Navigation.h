#pragma once
#include <Config.h>

class Navigation
{
public:
  /**
   * @brief Structure to store all sensor data
   */
  struct NAVIGATION_DATA
  {
    Gps_Wrapper::Gps_Data gps;
    Ranging_Wrapper::Ranging_Result ranging[3];
    Ranging_Wrapper::Position ranging_position;
  };

private:
  /**
   * @brief GPS object
   */
  Gps_Wrapper _gps;

  /**
   * @brief Ranging object
   */
  Ranging_Wrapper _ranging;
  int _slave_index = 0;
  int _last_slave_index = 0;

public:
  /**
   * @brief Structure to store all sensor data
   */
  NAVIGATION_DATA navigation_data;

  /**
   * @brief Required until the Ranging wrapper will be reewritten
  */
  bool ranging_initalized = false;

  /**
   * @brief Initialise the GPS
   * @param gps_config GPS config object
   * @return Whether the GPS was initialised successfully
   */
  bool beginGps(const Gps_Wrapper::Gps_Config_I2C &gps_config);

  /**
   * @brief Initialise the ranging
   * @param ranging_config Ranging config object
   * @param ranging_mode Ranging mode
   * @return Whether ranging was initialised successfully
   */
  bool beginRanging(const Ranging_Wrapper::Lora_Device &ranging_config, const Ranging_Wrapper::Mode &ranging_mode);

  /**
   * @brief Read the GPS data
   * @param data GPS data object
   * @return Whether the GPS data was read successfully
   * @bug To keep consistent with readRanging, I'm passing the entire navigation data structure
   * here as well, even though I can only pass the GPS data object
   */
  bool readGps(NAVIGATION_DATA &navigation_data);

  /**
   * @brief Read the ranging data
   * @param config Payload config object
   * @return Whether the ranging data was read successfully
   * @bug I couldn't pass the array of Ranging_wrapper::Ranging_Result to the function, so 
   * I had to pass the entire navigation data structure and then access the array from there
   */
  bool readRanging(Config &config, NAVIGATION_DATA &navigation_data);
};