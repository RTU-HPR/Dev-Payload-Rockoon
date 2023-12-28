#pragma once
#include <Config.h>
#include <Communication.h>
#include <Sensors.h>
#include <Navigation.h>
#include <Logging.h>

class Actions
{
private:
  // Prerequisite functions
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
  bool loggingActionEnabled = false;

  // Timed actions
  void runDataSendAction(Sensors &sensors, Navigation &navigation, Communication &communication, Config &config);
  bool dataSendActionEnabled = false;
  unsigned long lastDataSendActionMillis = 0;

  void runRangingAction(Navigation &navigation, Config &config);
  bool rangingSendActionEnabled = false;
  unsigned lastRangingActionMillis = 0;

  // Requested actions
  void runPongAction(Communication &communication, Config &config);
  bool pongActionEnabled = false;

  void runStatusAction(Communication &communication, Config &config);
  bool statusActionEnabled = false;

  void runMosfet1Action(Communication &communication, Config &config);
  bool mosfet1ActionEnabled = false;

  void runMosfet2Action(Communication &communication, Config &config);
  bool mosfet2ActionEnabled = false;

  void runDataRequestAction(Sensors &sensors, Navigation &navigation, Communication &communication, Config &config);
  bool dataRequestActionEnabled = false;

  void runRangingRequestAction(Navigation &navigation, Config &config);
  bool rangingRequestActionEnabled = false;

  void runContinousActions(Sensors &sensors, Navigation &navigation, Communication &communication, Logging &logging, Config &config);

  void runTimedActions(Sensors &sensors, Navigation &navigation, Communication &communication, Config &config);

  void runRequestedActions(Sensors &sensors, Navigation &navigation, Communication &communication, Config &config);

public:
  void runAllActions(Sensors &sensors, Navigation &navigation, Communication &communication, Logging &logging, Config &config);
};