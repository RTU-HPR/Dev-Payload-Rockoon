#include <Actions.h>

void Actions::runTimedActions(Sensors &sensors, Navigation &navigation, Communication &communication, Config &config)
{
  // Only start timed actions 10 seconds after turning on to make sure everything is initialised
  if (millis() < config.TIMED_ACTION_INITIAL_DELAY)
  {
    return;
  }

  // Using gps time and millis for redundancy purposes
  if (dataEssentialSendActionEnabled)
  {
    runEssentialDataSendAction(sensors, navigation, communication, config);
  }
}

void Actions::runEssentialDataSendAction(Sensors &sensors, Navigation &navigation, Communication &communication, Config &config)
{
  if (millis() - lastCommunicationCycle >= config.COMMUNICATION_ESSENTIAL_DATA_SEND_TIME && millis() - lastCommunicationCycle <= config.COMMUNICATION_CYCLE_INTERVAL)
  {
    String msg = createEssentialDataPacket(sensors, navigation, config);
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

String Actions::createEssentialDataPacket(Sensors &sensors, Navigation &navigation, Config &config)
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
  packet += "2500"; // Fix time since last gps, fix with actual data
  packet += ",";
  if (navigation.navigation_data.gps.epoch_time == 0)
  {
    packet += "0";
  }
  else
  {
    packet += "1";
  }
  packet += ",";
  packet += "0"; // Info/error in queue bool, fix with actual data

  return packet;
}