#include "Communication.h"

bool Communication::beginRadio(Config &config)
{
  // Create a radio object
  _radio = new RadioLib_Wrapper<radio_module>(nullptr, 5, "SX1268");

  // Initialize the radio
  if (!_radio->begin(config.radio_config))
  {
    return false;
  }

  return true;
}

void Communication::msgToUkhas(String &msg, Config &config)
{
  // Add the UKHAS prefix and the callsign to start of the message
  String ukhas_msg = "$$" + config.PFC_CALLSIGN + ",";
  ukhas_msg += msg;

  // Cheksum is calculated in the sendRadio function

  // Modify the original message
  msg = ukhas_msg;
}

bool Communication::sendRadio(String msg)
{
  String packet = msg;
  // This will add the checksum, endline and dashstar characters ("*xxxx\n")
  _radio->add_checksum(packet);
  // Send the message
  bool status = _radio->transmit(packet);
  return status;
}