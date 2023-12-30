#include <Logging.h>

bool Logging::begin(Config &config)
{
  if (!sd_card_wrapper.init(config.sd_card_config))
  {
    return false;
  }
  return true;
}

bool Logging::readConfig(Config &config)
{
  // String to store the read config
  String config_string;

  // Read the config file
  if (!sd_card_wrapper.read_config(config_string))
  {
    Serial.println("Config read failed!");

    // Create a new config file with default values
    writeConfig(config);

    return false;
  }

  // Check if the config file is empty
  // TODO: Would be nice to have a more robust check
  // As the parseString function can crash the mcu if the string is corrupt/invalid
  if (config_string.length() == 0)
  {
    Serial.println("Config file empty!");

    // Write the default config to the config file
    writeConfig(config);

    return false;
  }

  // Split the config string into an array of strings
  String config_string_values[CONFIG_FILE_VARIABLE_COUNT];
  parseString(config_string, config_string_values, CONFIG_FILE_VARIABLE_COUNT);
  // Convert the strings to the correct types and save to struct
  config.config_file_values.descent_flag = (config_string_values[0]).toInt();
  config.config_file_values.remaining_descent_time = (config_string_values[1]).toInt();
  config.config_file_values.parachutes_deployed_flag = (config_string_values[2]).toInt();
  config.config_file_values.heater_control_flag = (config_string_values[3]).toInt();

  return true;
}

void Logging::writeConfig(Config &config)
{
  // Convert the struct to a string
  String config_string = String(config.config_file_values.descent_flag) + "," +
                         String(config.config_file_values.remaining_descent_time) + "," +
                         String(config.config_file_values.parachutes_deployed_flag) + "," +
                         String(config.config_file_values.heater_control_flag);

  // Write the string to the config file
  if (!sd_card_wrapper.write_config(config_string, config.sd_card_config))
  {
    Serial.println("Config write failed!");
  }
  else
  {
    Serial.println("Config write successful!");
  }
}


// Function to parse a string containing comma-separated values as strings
void Logging::parseString(String &input, String *values, size_t maxSize)
{
  int startIndex = 0;
  int endIndex = input.indexOf(',');
  size_t index = 0;

  while (endIndex != -1 && index < maxSize)
  {
    // Extract each substring
    values[index] = input.substring(startIndex, endIndex);

    // Move to the next substring
    startIndex = endIndex + 1;
    endIndex = input.indexOf(',', startIndex);
    index++;
  }

  // Process the last substring
  if (index < maxSize)
  {
    values[index] = input.substring(startIndex);
  }
}

bool Logging::writeTelemetry(String &data)
{
  if (!sd_card_wrapper.get_initialized())
  {
    return false;
  }

  if (!sd_card_wrapper.write_data(data))
  {
    Serial.println("Telemetry write failed!");
    return false;
  }
  return true;
}

bool Logging::writeInfo(String &data)
{
  if (!sd_card_wrapper.get_initialized())
  {
    return false;
  }

  if (!sd_card_wrapper.write_info(data))
  {
    Serial.println("Info write failed!");
    return false;
  }
  return true;
}

bool Logging::writeError(String &data)
{
  if (!sd_card_wrapper.get_initialized())
  {
    return false;
  }

  if (!sd_card_wrapper.write_error(data))
  {
    Serial.println("Error write failed!");
    return false;
  }
  return true;
}