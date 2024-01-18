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
    runInfoErrorSendAction(communication, logging, navigation, config);
  }
  if (completeDataRequestActionEnabled)
  {
    runCompleteDataRequestAction(sensors, navigation, communication, heater, config);
  }
  if (formatStorageActionEnabled)
  {
    runFormatStorageAction(communication, logging, navigation, config);
  }
  if (pyroFireActionEnabled)
  {
    runPyroFireAction(communication, navigation, config);
  }
}

// Timed and Requested actions
void Actions::runInfoErrorSendAction(Communication &communication, Logging &logging, Navigation &navigation, Config &config)
{
  String msg_str = logging.readFromInfoErrorQueue();
  if (msg_str == "")
  {
    infoErrorRequestActionEnabled = false;
    Serial.println("No info errors to send");
    return;
  }

  uint16_t ccsds_packet_length;
  byte *ccsds_packet = create_ccsds_telemetry_packet(config.PFC_INFO_ERROR_RESPONSE, infoErrorResponseId, navigation.navigation_data.gps.epoch_time, 0, msg_str, ccsds_packet_length);

  // Send packet
  if (!communication.sendRadio(ccsds_packet, ccsds_packet_length))
  {
    // Do nothing for now
  }
  infoErrorResponseId++;
  infoErrorRequestActionEnabled = false;
  // Free memory
  delete[] ccsds_packet; // VERY IMPORTANT, otherwise a significant memory leak will occur
}

void Actions::runCompleteDataRequestAction(Sensors &sensors, Navigation &navigation, Communication &communication, Heater &heater, Config &config)
{
  String msg_str = createCompleteDataPacket(sensors, navigation, heater, config);

  uint16_t ccsds_packet_length;
  byte *ccsds_packet = create_ccsds_telemetry_packet(config.PFC_COMPLETE_DATA_RESPONSE, completeDataResponseId, navigation.navigation_data.gps.epoch_time, 0, msg_str, ccsds_packet_length);

  // Send packet
  if (!communication.sendRadio(ccsds_packet, ccsds_packet_length))
  {
    // Free memory after the packet has been sent
    delete[] ccsds_packet; // VERY IMPORTANT, otherwise a significant memory leak will occur
    return;
  }
  completeDataResponseId++;
  completeDataRequestActionEnabled = false;
  // Free memory
  delete[] ccsds_packet; // VERY IMPORTANT, otherwise a significant memory leak will occur
}

String Actions::createCompleteDataPacket(Sensors &sensors, Navigation &navigation, Heater &heater, Config &config)
{
  String packet = "";
  packet += String(sensors.data.containerBaro.temperature, 1);
  packet += ",";
  packet += String(sensors.data.containerTemperature.temperature, 1);
  packet += ",";
  packet += String(sensors.data.onBoardBaro.temperature, 1);
  packet += ",";
  packet += String(sensors.data.onBoardBaro.pressure);
  packet += ",";
  packet += String((uint32_t)heater.getHeaterPwm());
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

void Actions::runFormatStorageAction(Communication &communication, Logging &logging, Navigation &navigation, Config &config)
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
  String msg_str = String(success);

  uint16_t ccsds_packet_length;
  byte *ccsds_packet = create_ccsds_telemetry_packet(config.PFC_COMPLETE_DATA_RESPONSE, formatResponseId, navigation.navigation_data.gps.epoch_time, 0, msg_str, ccsds_packet_length);

  // Send packet
  if (!communication.sendRadio(ccsds_packet, ccsds_packet_length))
  {
    // Free memory after the packet has been sent
    delete[] ccsds_packet; // VERY IMPORTANT, otherwise a significant memory leak will occur
    return;
  }
  formatResponseId++;
  formatStorageActionEnabled = false;
  // Free memory
  delete[] ccsds_packet; // VERY IMPORTANT, otherwise a significant memory leak will occur
}

void Actions::runPyroFireAction(Communication &communication, Navigation &navigation, Config &config)
{
  String msg_str = "1"; // Success

  uint16_t ccsds_packet_length;
  byte *ccsds_packet = create_ccsds_telemetry_packet(config.PFC_COMPLETE_DATA_RESPONSE, pyroResponseId, navigation.navigation_data.gps.epoch_time, 0, msg_str, ccsds_packet_length);

  // Send packet
  if (!communication.sendRadio(ccsds_packet, ccsds_packet_length))
  {
    // Free memory after the packet has been sent
    delete[] ccsds_packet; // VERY IMPORTANT, otherwise a significant memory leak will occur
    return;
  }
  pyroResponseId++;
  // Free memory
  delete[] ccsds_packet; // VERY IMPORTANT, otherwise a significant memory leak will occur
}