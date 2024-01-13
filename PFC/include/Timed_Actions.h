#include <Actions.h>

void Actions::runTimedActions(Sensors &sensors, Navigation &navigation, Communication &communication, Logging &logging, Config &config)
{
  // Only start timed actions 10 seconds after turning on to make sure everything is initialised
  if (millis() < config.TIMED_ACTION_INITIAL_DELAY)
  {
    return;
  }

  // Using gps time and millis for redundancy purposes
  if (dataEssentialSendActionEnabled)
  {
    runEssentialDataSendAction(sensors, navigation, communication, logging, config);
  }
}

void Actions::runEssentialDataSendAction(Sensors &sensors, Navigation &navigation, Communication &communication, Logging &logging, Config &config)
{
  if (millis() - lastCommunicationCycle >= config.COMMUNICATION_ESSENTIAL_DATA_SEND_TIME && millis() - lastCommunicationCycle <= config.COMMUNICATION_CYCLE_INTERVAL)
  {
    String msg = createEssentialDataPacket(sensors, navigation, logging, config);
    communication.msgToUkhas(msg, config);
    if (!communication.sendRadio(msg))
    {
      return;
    }
    dataEssentialResponseId++;
    dataEssentialSendActionEnabled = false;
    Serial.println("Sending essential data");
  }
}

String Actions::createEssentialDataPacket(Sensors &sensors, Navigation &navigation, Logging &logging, Config &config)
{
  String packet = "";
  packet += config.PFC_ESSENTIAL_DATA_RESPONSE;
  packet += ",";
  packet += String(dataEssentialResponseId);
  packet += ",";
  packet += String(navigation.navigation_data.gps.lat, 6);
  packet += ",";
  packet += String(navigation.navigation_data.gps.lng, 6);
  packet += ",";
  packet += String(navigation.navigation_data.gps.altitude, 2);
  packet += ",";
  packet += String(sensors.data.onBoardBaro.altitude, 2);
  packet += ",";
  packet += String(navigation.navigation_data.gps.satellites);
  packet += ",";
  packet += String(navigation.navigation_data.gps.epoch_time);
  packet += ",";
  packet += String(!logging.infoErrorQueueEmpty());

  return packet;
}