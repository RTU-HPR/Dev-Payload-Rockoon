#pragma once
#include <RadioLib_wrapper.h>
#include <Config.h>

class Communication
{
  private:
    
  public:
    RadioLib_Wrapper<radio_module> *_radio;

    /**
     * @brief Structure to store received message from the Radio or PC
    */
    struct RECEIVED_MESSAGE_STRUCTURE
    {
      String msg;
      float rssi;
      float snr;
      double frequency;
      bool processed;
      bool checksum_good;
      bool radio_message;
    };

    RECEIVED_MESSAGE_STRUCTURE received;

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

    /**
     * @brief Check for received commands from Radio or PC
     * @param received The structure where to store the received message
     * @return Whether a message was received
    */
    bool receiveCommand(RECEIVED_MESSAGE_STRUCTURE &received);
};
