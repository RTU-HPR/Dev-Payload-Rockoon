#pragma once
#include <Config.h>
#include <Sd_card_wrapper.h>

class Logging
{
private:
  SD_Card_Wrapper sd_card_wrapper = SD_Card_Wrapper(nullptr, "SD Card");;

  static const int CONFIG_FILE_VARIABLE_COUNT = 4;
  void writeConfig(Config &config);

public:
  bool begin(Config &config);
  bool readConfig(Config &config);

  bool writeTelemetry(String &data);
  bool writeInfo(String &data);
  bool writeError(String &data);
  
  void parseString(String &input, String *values, size_t maxSize);
};