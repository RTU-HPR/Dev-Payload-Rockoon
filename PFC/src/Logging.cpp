#include <Logging.h>

bool Logging::begin(Config &config)
{
  sd_card_wrapper = SD_Card_Wrapper(nullptr, "SD Card");;
  if (!sd_card_wrapper.init(config.sd_card_config))
  {
    Serial.println("SD card initialization failed!");
    return false;
  }
  Serial.println("SD card initialization complete");
  return true;
}

bool Logging::writeTelemetry(String data)
{
  if (!sd_card_wrapper.write_data(data))
  {
    Serial.println("Telemetry write failed!");
    return false;
  }
  return true;
}

bool Logging::writeInfo(String data)
{
  if (!sd_card_wrapper.write_info(data))
  {
    Serial.println("Info write failed!");
    return false;
  }
  return true;
}

bool Logging::writeError(String data)
{
  if (!sd_card_wrapper.write_error(data))
  {
    Serial.println("Error write failed!");
    return false;
  }
  return true;
}