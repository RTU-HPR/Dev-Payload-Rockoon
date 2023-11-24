#pragma once
#include <RadioLib.h>
#include <Arduino.h>

namespace RadioLib_interrupts
{
    /**
     * @brief Set the flag that a message has been received
     */
    void set_receive_flag(void);

    /**
     * @brief Set the flag that a message has been sent
     */
    void set_transmit_flag(void);
}

template <typename T>
class RadioLib_Wrapper
{
private:
    /**
     * @brief Return a string with the name of the used radio type
     *
     * https://stackoverflow.com/questions/75005021/how-to-retrieve-the-data-type-of-a-variable
     *
     * @tparam T Class name of Radio module used
     * @return String Name of the radio module used
     */
    String type_name();

public:
    // Radio object
    T radio = new Module(-1, -1, -1, -1);

    // Radio module name
    String radio_typename = "";

    // Radio state
    struct state
    {
        bool initialized = false;
        int action_status_code = RADIOLIB_ERR_NONE;
    };
    state state;

    /**
     * @brief Creates a new RadioLib wrapper and initialize the radio module
     *
     * @param CS Chip select
     * @param DIO0 DIO0 pin (Busy)
     * @param RESET Reset pin
     * @param DIO1 DIO1 pin
     */
    RadioLib_Wrapper(int CS, int DIO0, int RESET, int DIO1, HardwareSPI *SPI);

    /**
     * @brief Configure radio module with given settings
     *
     * @param FREQUENCY Frequency to be used
     * @param TXPOWER Tranmission power
     * @param SPREADING Spreading factor
     * @param CODING_RATE Coding rate
     * @param SIGNAL_BW Signal bandwidth
     * @param SYNC_WORD Sync word
     * @return true If configured successfully
     * @return false If not configured successfully
     */
    bool configure_radio(float FREQUENCY, int TXPOWER, int SPREADING, int CODING_RATE, float SIGNAL_BW, int SYNC_WORD);

    /**
     * @brief Return radio initialization status
     *
     * @return true If radio is initialized
     * @return false If radio is not initialized
     */
    bool status();

    /**
     * @brief Send a message over the radio
     *
     * @param msg Message to send
     * @return true If transmit was successful
     * @return false If transmit failed
     */
    bool transmit(String msg);

    /**
     * @brief Read any received data
     *
     * @param msg Reference to variable where to save the message
     * @param rssi Reference to variable where to save the message RSSI
     * @param snr Reference to variable where to save the message SNR
     * @return true If a message was received
     * @return false If receive failed or no message was received
     */
    bool receive(String &msg, float &rssi, float &snr);

    /**
     * @brief Transmit a test message
     *
     * @return true If transmit was successful
     * @return false If transmit failed
     */
    bool test_transmit();
};

// Selected SX12xx LoRa types
template class RadioLib_Wrapper<SX1262>;
template class RadioLib_Wrapper<SX1268>;
template class RadioLib_Wrapper<SX1272>;
template class RadioLib_Wrapper<SX1273>;
template class RadioLib_Wrapper<SX1276>;
template class RadioLib_Wrapper<SX1277>;
template class RadioLib_Wrapper<SX1278>;
template class RadioLib_Wrapper<SX1279>;
template class RadioLib_Wrapper<SX1280>;
template class RadioLib_Wrapper<SX1281>;
template class RadioLib_Wrapper<SX1282>;

// Selected RFM9x LoRa types
template class RadioLib_Wrapper<RFM95>;
template class RadioLib_Wrapper<RFM96>;
