#pragma once
#include <Config.h>

class Logging
{
private:
  static const int CONFIG_FILE_VARIABLE_COUNT = 4;

  SD_Card_Wrapper sd_card_wrapper = SD_Card_Wrapper(nullptr, "SD Card");

  /**
   * @brief Structure to store info and error messages. Required for the queue
   */

  typedef struct info_error
  {
    char message[INFO_ERROR_MAX_LENGTH];
  } Info_Error_Message;

  /**
   * @brief Queue to store info and error messages
   */
  cppQueue *infoErrorQueue; // Queue to store info and error messages

  /**
   * @brief Write a string to the info file
   */
  bool writeInfo(String &data);

  /**
   * @brief Write a string to the config file
   */
  void writeConfig(Config &config);

public:
  /**
   * @brief Initialise the logging
   * @param config Config object
   * @return Whether the logging was initialised successfully
   */
  bool begin(Config &config);

  /**
   * @brief Read the config file
   * @param config Config object
   * @return Whether the config file was read successfully
   */
  bool readConfig(Config &config);

  /**
   * @brief Add a string to the info/error queue
   * @param message String to add
   * @return Whether the string was added successfully
   */
  bool addToInfoErrorQueue(String &message);

  /**
   * @brief Read a string from the info/error queue
   * @return String read from the queue
   */
  String readFromInfoErrorQueue();

  /**
   * @brief Check if the info/error queue is empty
   * @return Whether the info/error queue is empty
   */
  bool infoErrorQueueEmpty();

  /**
   * @brief Record an info message
   * @param info Info message
   * @return Whether the info message was recorded successfully
   */
  bool recordInfo(String &info);
  
  /**
   * @brief Record an error message
   * @param error Error message
   * @return Whether the error message was recorded successfully
   */
  bool recordError(String &error);

  /**
   * @brief Record a telemetry message
   * @param data Telemetry data
   * @return Whether the telemetry message was recorded successfully
   */
  bool writeTelemetry(String &data);

  /**
   * @brief Write a string to the error file
   */
  bool writeError(String &data);

  /**
   * @brief Parse a string into an array of strings
   * @param input String to parse
   * @param values Array to store the parsed strings
   * @param maxSize Maximum number of strings to parse
   */
  void parseString(String &input, String *values, size_t maxSize);

  /**
   * @brief Format the SD card
   * @param config Config object
   * @return Whether the SD card was formatted successfully
   */
  bool formatSdCard(Config &config);
};