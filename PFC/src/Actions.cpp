#include <Actions.h>
#include <Config.h>

void Actions::runAllActions(Sensors &sensors, Navigation &navigation, Communication &communication, Logging &logging)
{
  if (sensorActionEnabled)
  {
    runSensorAction(sensors);
  }

  if (navigationActionEnabled)
  {
    runNavigationAction(navigation);
  }

  if (radioActionEnabled)
  {
    runRadioAction(communication);
  }

  if (loggingActionEnabled)
  {
    runLoggingAction(logging);
  }
}

void Actions::runRadioAction(Communication &communication)
{
}

void Actions::runSensorAction(Sensors &sensors)
{
  // Read all sensors
  sensors.readSensors();
}

void Actions::runNavigationAction(Navigation &navigation)
{
  navigation.readGps(navigation.navigation_data);
}

void Actions::runLoggingAction(Logging &logging)
{
  logging.writeTelemetry("test");
  logging.writeInfo("test");
  logging.writeError("test");
}