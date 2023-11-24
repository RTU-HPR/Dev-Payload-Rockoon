#include "RadioLib_wrapper.h"

// Flags for radio interrupt functions
volatile bool received_flag = false;
volatile bool transmitted_flag = false;

/*
If compiling for ESP boards, specify that these function are used within interrupt routine
and such should be stored in the RAM and not the flash memory
*/
#if defined(ESP8266) || defined(ESP32)
ICACHE_RAM_ATTR
#endif
void RadioLib_interrupts::set_receive_flag(void)
{
    received_flag = true;
}

#if defined(ESP8266) || defined(ESP32)
ICACHE_RAM_ATTR
#endif
void RadioLib_interrupts::set_transmit_flag(void)
{
    transmitted_flag = true;
}

template <typename T>
String RadioLib_Wrapper<T>::type_name()
{
    String s = __PRETTY_FUNCTION__;
    int start = s.indexOf("[with T = ") + 10;
    int stop = s.lastIndexOf(']');
    return s.substring(start, stop);
}

template <typename T>
RadioLib_Wrapper<T>::RadioLib_Wrapper(int CS, int DIO0, int RESET, int DIO1)
{
    // Create new LoRa object
    radio = new Module(CS, DIO0, RESET, DIO1);

    // Save the name of the radio type
    radio_typename = type_name();

    // Try to initialize communication with LoRa
    state.action_status_code = radio.begin();

    // If initialization failed, print error
    if (state.action_status_code != RADIOLIB_ERR_NONE)
    {
        Serial.println(radio_typename + " initialization failed with status code: " + String(state.action_status_code));
        return;
    }

    // Set interrupt behaviour
    radio.setPacketReceivedAction(set_receive_flag);
    radio.setPacketSentAction(set_transmit_flag);

    // Start receiving with radio module
    state.action_status_code = radio.startReceive();
    if (state.action_status_code != RADIOLIB_ERR_NONE)
    {
        Serial.println(radio_typename + " Receive start failed with status code: " + String(state.action_status_code));
        return;
    }

    // Set that radio has been initialized
    state.initialized = true;
}

template <typename T>
bool RadioLib_Wrapper<T>::configure_radio(float FREQUENCY, int TXPOWER, int SPREADING, int CODING_RATE, float SIGNAL_BW, int SYNC_WORD)
{
    if (radio.setFrequency(FREQUENCY) == RADIOLIB_ERR_INVALID_FREQUENCY)
    {
        Serial.println(radio_typename + " Frequency is invalid: " + String(FREQUENCY));
        return false;
    };

    if (radio.setOutputPower(TXPOWER) == RADIOLIB_ERR_INVALID_OUTPUT_POWER)
    {
        Serial.println(radio_typename + " Transmit power is invalid: " + String(TXPOWER));
        return false;
    };

    if (radio.setSpreadingFactor(SPREADING) == RADIOLIB_ERR_INVALID_SPREADING_FACTOR)
    {
        Serial.println(radio_typename + " Spreading factor is invalid: " + String(SPREADING));
        return false;
    };

    if (radio.setCodingRate(CODING_RATE) == RADIOLIB_ERR_INVALID_CODING_RATE)
    {
        Serial.println(radio_typename + " Coding rate is invalid: " + String(CODING_RATE));
        return false;
    };

    if (radio.setBandwidth(SIGNAL_BW) == RADIOLIB_ERR_INVALID_BANDWIDTH)
    {
        Serial.println(radio_typename + " Signal bandwidth is invalid: " + String(SIGNAL_BW));
        return false;
    };

    if (radio.setSyncWord(SYNC_WORD) == RADIOLIB_ERR_INVALID_SYNC_WORD)
    {
        Serial.println(radio_typename + " Sync word is invalid: " + String(SYNC_WORD));
        return false;
    };

    return true;
}

template <typename T>
bool RadioLib_Wrapper<T>::status()
{
    return state.initialized;
}

template <typename T>
bool RadioLib_Wrapper<T>::transmit(String msg)
{
    if (!transmitted_flag || !state.initialized)
    {
        return false;
    }
    else
    {
        // Reset the flag
        transmitted_flag = false;
    }

    // Start transmitting
    state.action_status_code = radio.startTransmit(msg);

    // If transmit failed, print error
    if (state.action_status_code != RADIOLIB_ERR_NONE)
    {
        Serial.println(radio_typename + " Starting transmit failed with status code:" + String(state.action_status_code));
        return false;
    }

    const int MAX_WAIT = 1000;
    unsigned long int last_millis = millis();
    while (!transmitted_flag || (millis() - last_millis > MAX_WAIT))
    {
        delay(1);
        last_millis = millis();
    }

    if (state.action_status_code != RADIOLIB_ERR_NONE)
    {
        Serial.println(radio_typename + " Transmit failed with status code: " + String(state.action_status_code));
        return false;
    }

    // Clean up after transmission is finished
    radio.finishTransmit();

    return true;
}

// Listen to messages over LoRa. Returns true if received successfully
template <typename T>
bool RadioLib_Wrapper<T>::receive(String &msg, float &rssi, float &snr)
{
    if (!received_flag || !state.initialized)
    {
        return false;
    }
    else
    {
        // Reset the flag
        received_flag = false;
    }

    // Try to read received data
    String str = "";
    state.action_status_code = radio.readData(str);

    if (state.action_status_code != RADIOLIB_ERR_NONE)
    {
        Serial.println(radio_typename + " Receiving failed with status code: " + String(state.action_status_code));
    }

    msg = str;
    rssi = radio.getRSSI();
    snr = radio.getSNR();

    // Restart receiving
    radio.startReceive();

    if (msg == "")
    {
        return false;
    }

    return true;
}

template <typename T>
bool RadioLib_Wrapper<T>::test_transmit()
{
    String msg = radio_typename + " Transmission test";

    // Try to transmit the test message
    if (radio.transmit(msg))
    {
        Serial.println(radio_typename + " Test transmission failed");
        Serial.println("Setting radio as not initialized");
        return false;
    }

    Serial.println(radio_typename + " Test transmission was successful");
    return true;
}