#pragma once
#include <Config.h>
#include <Sd_card_wrapper.h>

class Logging
{
private:
  SD_Card_Wrapper sd_card_wrapper;

public:
  bool begin(Config &config);

  bool writeTelemetry(String data);
  bool writeInfo(String data);
  bool writeError(String data);
};