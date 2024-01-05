#pragma once
#include <Config.h>
#include <Communication.h>
#include <Sensors.h>
#include <Navigation.h>
#include <Logging.h>

// Get the global loop time variable
extern int loopTime;

class Actions
{
private:
    // Prerequisite functions
    String createStatusPacket(Sensors &sensors, Navigation &navigation, Config &config);
    unsigned int status_packet_id = 1;

    String createSendablePacket(Sensors &sensors, Navigation &navigation);
    unsigned int sendable_packet_id = 1;

    String createLoggablePacket(Sensors &sensors, Navigation &navigation);
    unsigned long loggable_packed_id = 1;

    // Continuous actions
    void runCommandReceiveAction(Communication &communication, Config &config);
    bool commandReceiveActionEnabled = true;

    void runSensorAction(Sensors &sensors);
    bool sensorActionEnabled = true;

    void runGpsAction(Navigation &navigation);
    bool gpsActionEnabled = true;

    void runLoggingAction(Logging &logging, Navigation &navigation, Sensors &sensors);
    bool loggingActionEnabled = true;

    void runRangingAction(Navigation &navigation, Config &config);
    bool rangingSendActionEnabled = true;

    void runGetCommunicationCycleStartAction(Navigation &navigation, Config &config);
    bool getCommunicationCycleStartActionEnabled = true;
    unsigned long lastCommunicationCycle = 0;

    // Timed actions
    void runEssentialDataSendAction(Sensors &sensors, Navigation &navigation, Communication &communication, Config &config);
    bool dataEssentialSendActionEnabled = true;

    // Timed and Requested actions
    bool requestedActionEnabled = true;

    void runInfoErrorSendAction(Communication &communication, Logging &logging);
    bool infoErrorRequestActionEnabled = true;

    void runCompleteDataRequestAction(Sensors &sensors, Navigation &navigation, Communication &communication, Config &config);
    bool compleateDataRequestActionEnabled = true;

    void runFormatStorageAction(Communication &communication, Logging &logging, Config &config);
    bool formatStorageActionEnabled = true;

    void runHeaterSetAction(Communication &communication, Config &config);
    bool heaterSetActionEnabled = true;

    void runPyroFireAction(Communication &communication, Config &config);
    bool pyroFireActionEnabled = true;

    // void runStatusAction(Sensors &sensors, Navigation &navigation, Communication &communication, Config &config);
    // bool statusActionEnabled = false;

    // void runMosfet1Action(Communication &communication, Config &config);
    // bool mosfet1ActionEnabled = false;

    // void runMosfet2Action(Communication &communication, Config &config);
    // bool mosfet2ActionEnabled = false;

    // void runDataRequestAction(Sensors &sensors, Navigation &navigation, Communication &communication, Config &config);
    // bool dataRequestActionEnabled = false;

    // void runRangingRequestAction(Navigation &navigation, Config &config);
    // bool rangingRequestActionEnabled = false;

    void runContinousActions(Sensors &sensors, Navigation &navigation, Communication &communication, Logging &logging, Config &config);

    void runTimedActions(Sensors &sensors, Navigation &navigation, Communication &communication, Config &config);

    void runRequestedActions(Sensors &sensors, Navigation &navigation, Communication &communication, Config &config);

public:
    void runAllActions(Sensors &sensors, Navigation &navigation, Communication &communication, Logging &logging, Config &config);
};