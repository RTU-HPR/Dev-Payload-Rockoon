#include <Actions.h>

void Actions::runRequestedActions(Sensors &sensors, Navigation &navigation, Communication &communication, Logging &logging, Heater &heater, Config &config)
{
  // Check if the communication cycle is within the response send time
  if (!(millis() - lastCommunicationCycle >= config.COMMUNICATION_RESPONSE_SEND_TIME && millis() - lastCommunicationCycle <= config.COMMUNICATION_ESSENTIAL_DATA_SEND_TIME))
  {
    return;
  }

  if (infoErrorRequestActionEnabled)
  {
    runInfoErrorSendAction(communication, logging, config);
  }
  if (completeDataRequestActionEnabled)
  {
    runCompleteDataRequestAction(sensors, navigation, communication, heater, config);
  }
  if (formatStorageActionEnabled)
  {
    runFormatStorageAction(communication, logging, config);
  }
  if (pyroFireActionEnabled)
  {
    runPyroFireAction(communication, config);
  }
}

// Timed and Requested actions
void Actions::runInfoErrorSendAction(Communication &communication, Logging &logging, Config &config)
{
  String infoErrorMSG = logging.readFromInfoErrorQueue();
  String msg = config.PFC_INFO_ERROR_RESPONSE + "," + infoErrorResponseId + "," + infoErrorMSG;
  communication.msgToUkhas(msg, config);
  if (!communication.sendRadio(msg))
  {
    return;
  }
  infoErrorResponseId++;
  infoErrorRequestActionEnabled = false;
}

void Actions::runCompleteDataRequestAction(Sensors &sensors, Navigation &navigation, Communication &communication, Heater &heater, Config &config)
{
  String msg = createCompleteDataPacket(sensors, navigation, heater, config);
  communication.msgToUkhas(msg, config);
  if (!communication.sendRadio(msg))
  {
    return;
  }
  completeDataResponseId++;
  completeDataRequestActionEnabled = false;
}

String Actions::createCompleteDataPacket(Sensors &sensors, Navigation &navigation, Heater &heater, Config &config)
{
  String packet = "";
  packet += config.PFC_COMPLETE_DATA_RESPONSE;
  packet += ",";
  packet += String(completeDataResponseId);
  packet += ",";
  packet += String(sensors.data.containerBaro.temperature, 1);
  packet += ",";
  packet += String(sensors.data.containerTemperature.temperature, 1);
  packet += ",";
  packet += String(sensors.data.onBoardBaro.temperature, 1);
  packet += ",";
  packet += String(sensors.data.onBoardBaro.pressure);
  packet += ",";
  packet += String(heater.getHeaterPwm());
  packet += ",";
  packet += String(navigation.navigation_data.ranging[0].distance, 1);
  packet += ",";
  packet += String(navigation.navigation_data.ranging[1].distance, 1);
  packet += ",";
  packet += String(navigation.navigation_data.ranging[0].time);
  packet += ",";
  packet += String(navigation.navigation_data.ranging[1].time);
  packet += ",";
  packet += String(sensors.data.battery.voltage, 2);

  return packet;
}

void Actions::runFormatStorageAction(Communication &communication, Logging &logging, Config &config)
{
  // Format the SD card
  bool success;
  if (logging.formatSdCard(config))
  {
    success = true;
  }
  else
  {
    success = false;
  }

  // Send the response
  String msg = config.PFC_FORMAT_RESPONSE + "," + formatResponseId + "," + String(success);
  communication.msgToUkhas(msg, config);
  if (!communication.sendRadio(msg))
  {
    return;
  }
  formatResponseId++;
  formatStorageActionEnabled = false;
}

void Actions::runPyroFireAction(Communication &communication, Config &config)
{
  String msg = config.PFC_PYRO_RESPONSE + "," + pyroResponseId + "," + "1";
  communication.msgToUkhas(msg, config);
  if (!communication.sendRadio(msg))
  {
    return;
  }
  pyroResponseId++;
  infoErrorRequestActionEnabled = false;
}