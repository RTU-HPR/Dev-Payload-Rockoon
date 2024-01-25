#pragma once
#include <Config.h>
#include <Communication.h>
#include <Sensors.h>
#include <Navigation.h>
#include <Logging.h>
#include <Heater.h>

// Get the global loop time variable
extern int loopTime;

class Actions
{
private:
    // Prerequisite functions
    String createEssentialDataPacket(Sensors &sensors, Navigation &navigation, Logging &logging, Config &config);
    String createCompleteDataPacket(Sensors &sensors, Navigation &navigation, Heater &heater, Config &config);
    String createLoggablePacket(Sensors &sensors, Navigation &navigation);
    unsigned long loggable_packed_id = 1;

    // Continuous actions
    void runContinousActions(Sensors &sensors, Navigation &navigation, Communication &communication, Logging &logging, Heater &heater, Config &config);

    void runCommandReceiveAction(Communication &communication, Logging &logging, Config &config);
    bool commandReceiveActionEnabled = true;

    void runSensorAction(Sensors &sensors);
    bool sensorActionEnabled = true;

    void runHeaterAction(Heater &heater, const Sensors &sensors);
    bool heaterActionEnabled = true;

    void runGpsAction(Navigation &navigation);
    bool gpsActionEnabled = true;

    void runLoggingAction(Logging &logging, Navigation &navigation, Sensors &sensors);
    bool loggingActionEnabled = true;

    void runRangingAction(Navigation &navigation, Config &config);
    bool rangingSendActionEnabled = true;

    void runGetCommunicationCycleStartAction(Navigation &navigation, Config &config);
    bool getCommunicationCycleStartActionEnabled = true;
    unsigned long lastCommunicationCycle = 0;

    void runPyroChannelManagerAction(Config &config);
    bool pyroChannelManagerActionEnabled = true;
    bool pyroChannelShouldBeFired[2] = {false, false};
    unsigned long pyroChannelFireTimes[2] = {0, 0};

    // Timed actions
    void runTimedActions(Sensors &sensors, Navigation &navigation, Communication &communication, Logging &logging, Config &config);

    void runEssentialDataSendAction(Sensors &sensors, Navigation &navigation, Communication &communication, Logging &logging, Config &config);
    bool dataEssentialSendActionEnabled = true;
    uint16_t dataEssentialResponseId = 0;

    // Requested actions
    void runRequestedActions(Sensors &sensors, Navigation &navigation, Communication &communication, Logging &logging, Heater &heater, Config &config);

    void runInfoErrorSendAction(Communication &communication, Logging &logging, Navigation &navigation, Config &config);
    bool infoErrorRequestActionEnabled = false;
    uint16_t infoErrorResponseId = 0;

    void runCompleteDataRequestAction(Sensors &sensors, Navigation &navigation, Communication &communication, Heater &heater, Config &config);
    bool completeDataRequestActionEnabled = false;
    uint16_t completeDataResponseId = 0;

    void runFormatStorageAction(Communication &communication, Logging &logging, Navigation &navigation, Config &config);
    bool formatStorageActionEnabled = false;
    uint16_t formatResponseId = 0;

    void runPyroFireAction(Communication &communication, Navigation &navigation, Config &config);
    bool pyroFireActionEnabled = false;
    uint16_t pyroResponseId = 0;

public:
    void runAllActions(Sensors &sensors, Navigation &navigation, Communication &communication, Logging &logging, Heater &heater, Config &config);
};