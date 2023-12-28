#pragma once
#include <Config.h>
#include <Communication.h>
#include <Sensors.h>
#include <Navigation.h>
#include <Logging.h>

class Actions
{
  private:

  public:
  void runAllActions(Sensors &sensors, Navigation &navigation, Communication &communication, Logging &logging);
  
  void runRadioAction(Communication &communication);
  bool radioActionEnabled = false;

  void runSensorAction(Sensors &sensors);
  bool sensorActionEnabled = true;

  void runNavigationAction(Navigation &navigation);
  bool navigationActionEnabled = false;

  void runLoggingAction(Logging &logging);
  bool loggingActionEnabled = true;
};