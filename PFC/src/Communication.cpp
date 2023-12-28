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
  // This will add the checksum, endline and dashstar characters ("*xxxx\n")
  _radio->add_checksum(msg);
  
  // Send the message 
  bool status = _radio->transmit(msg);
  return status;
}

bool Communication::receiveCommand(RECEIVED_MESSAGE_STRUCTURE &received)
{
  // Check for any messages from Radio
  if (_radio->receive(received.msg, received.rssi, received.snr, received.frequency))
  {
    // Check if checksum matches
    if (_radio->check_checksum(received.msg))
    {
      received.checksum_good = true;
    }
    else
    {
      received.checksum_good = false;
    }

    // Set the flags
    received.processed = false;
    received.radio_message = true;

    return true;
  }

  // Check for any messages from PC
  if (Serial.available() > 0)
  {
    // Read the message from the Serial port
    received.msg = Serial.readString();
    // Remove any line ending symbols
    received.msg.trim();

    // Set the flags
    received.checksum_good = true;
    received.processed = false;
    received.radio_message = false;

    return true;
  }

  return false;
}