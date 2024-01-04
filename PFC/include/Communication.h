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
     * @brief Convert the message to the UKHAS format
     * @details
     * - Used refrences:
     *  - https://habitat.readthedocs.io/en/latest/ukhas_parser.html
     *  - https://ukhas.org.uk/doku.php?id=communication:protocol
     * @param msg Message to modify
     * @param config Payload config object
    */
    void msgToUkhas(String &msg, Config &config);

    /**
     * @brief Sends the provided message using LoRa
     * @param msg Message to send
     * @return Whether the message was sent successfully
    */
    bool sendRadio(String msg);

    bool sendError(String errorString);
};
