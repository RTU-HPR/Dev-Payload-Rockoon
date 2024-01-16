#pragma once
#include <RadioLib_wrapper.h>
#include <Config.h>

class Communication
{
  private:

  public:
    RadioLib_Wrapper<radio_module> *_radio;

    /**
     * @brief Initialise the Communication Radio
     * @param config Payload config object
    */
    bool beginRadio(Config &config);

    /**
     * @brief Sends the provided message using LoRa
     * @param msg Message to send
     * @return Whether the message was sent successfully
    */
    bool sendRadio(byte *ccsds_packet, uint16_t ccsds_packet_length);

    bool sendError(String errorString);
};
