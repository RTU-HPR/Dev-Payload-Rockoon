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

bool Communication::sendRadio(byte *ccsds_packet, uint16_t ccsds_packet_length)
{
  if (!_radio->get_initialized())
  {
    return false;
  }
  // Send the message
  bool status = _radio->transmit_bytes(ccsds_packet, ccsds_packet_length);
  return status;
}

bool Communication::sendError(String errorString)
{
  // // Add the error prefix to the error string
  // errorString = "PFC ERROR: " + errorString;

  // // Send the error message
  // return sendRadio();
  return true;
}