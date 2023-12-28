#include <Actions.h>
#include <Config.h>

void Actions::runAllActions(Sensors &sensors, Navigation &navigation, Communication &communication, Logging &logging, Config &config)
{
  // Receive commands, read sensors and gps, log data to sd card
  runContinousActions(sensors, navigation, communication, logging, config);

  // Do ranging and send telemetry data
  runTimedActions(sensors, navigation, communication, config);

  // Do actions requested by a command
  runRequestedActions(sensors, navigation, communication, config);
}

void Actions::runContinousActions(Sensors &sensors, Navigation &navigation, Communication &communication, Logging &logging, Config &config)
{
  // Run the command receive action
  if (commandReceiveActionEnabled)
  {
    runCommandReceiveAction(communication, config);
  }
  // Run the sensor action
  if (sensorActionEnabled)
  {
    runSensorAction(sensors);
  }
  // Run the navigation action
  if (gpsActionEnabled)
  {
    runGpsAction(navigation);
  }
  // Run the logging action
  if (loggingActionEnabled)
  {
    runLoggingAction(logging, navigation, sensors);
  }
}

void Actions::runTimedActions(Sensors &sensors, Navigation &navigation, Communication &communication, Config &config)
{
  // Only start timed actions 10 seconds after turning on for safety purposes
  if (millis() < config.TIMED_ACTION_INITIAL_DELAY)
  {
    return;
  }

  // Run the ranging the set time before data send action
  if (rangingSendActionEnabled && (millis() - lastRangingActionMillis >= config.RANGING_ACTION_SAFETY_INTERVAL) && (millis() - (lastDataSendActionMillis - config.RANGING_ACTION_PREEMPTIVE_INTERVAL) >= config.DATA_SEND_ACTION_INTERVAL))
  {
    runRangingAction(navigation, config);
    lastRangingActionMillis = millis();
  }

  // Using gps time and millis for redundancy purposes
  // Run the data send action, if it is enabled, GPS secoconds are dividiable by 15 or atleast 15,5 seconds have passed since the last transmit
  if (dataSendActionEnabled && ((navigation.navigation_data.gps.second % 15 == 0) || (millis() - (lastDataSendActionMillis + 500) >= config.DATA_SEND_ACTION_INTERVAL)))
  {
    runDataSendAction(sensors, navigation, communication, config);
    lastDataSendActionMillis = millis();
  }
}

void Actions::runRequestedActions(Sensors &sensors, Navigation &navigation, Communication &communication, Config &config)
{
  if (pongActionEnabled)
  {
    runPongAction(communication, config);
  }
  if (statusActionEnabled)
  {
    runStatusAction(communication, config);
  }
  if (mosfet1ActionEnabled)
  {
    runMosfet1Action(communication, config);
  }
  if (mosfet2ActionEnabled)
  {
    runMosfet2Action(communication, config);
  }
  if (dataRequestActionEnabled)
  {
    runDataRequestAction(sensors, navigation, communication, config);
  }
  if (rangingRequestActionEnabled)
  {
    runRangingAction(navigation, config);
  }
}

void Actions::runCommandReceiveAction(Communication &communication, Config &config)
{
  String msg = "";
  float rssi = 1;
  float snr = 0;
  double frequency = 0;
  bool checksum_good = false;

  // Check for any messages from Radio
  if (communication._radio->receive(msg, rssi, snr, frequency))
  {
    // Check if checksum matches
    if (communication._radio->check_checksum(msg))
    {
      checksum_good = true;
    }
  }

  // Check for any messages from PC
  if (Serial.available() > 0)
  {
    // Read the message from the Serial port
    msg = Serial.readString();
    // Remove any line ending symbols
    msg.trim();

    // Set the flags
    checksum_good = true;
  }

  // Check if the message is not empty and the checksum is good
  if (msg != "" && checksum_good)
  {
    // Print the received message
    if (rssi < 0)
    {
      Serial.print("RADIO COMMAND | RSSI: " + String(rssi) + " | SNR: " + String(snr) + " FREQUENCY: " + String(frequency, 8) + " | MSG: ");
    }
    else
    {
      Serial.print("PC COMMAND | MSG: ");
    }
    Serial.println(msg);

    // Set the action flag according to the received command
    if (msg == config.PFC_PING)
    {
      pongActionEnabled = true;
    }
    else if (msg == config.PFC_STATUS)
    {
      statusActionEnabled = true;
    }
    else if (msg == config.PFC_MOSFET_1)
    {
      mosfet1ActionEnabled = true;
    }
    else if (msg == config.PFC_MOSFET_2)
    {
      mosfet2ActionEnabled = true;
    }
    else if (msg == config.PFC_DATA_REQUEST)
    {
      dataRequestActionEnabled = true;
    }
    else if (msg == config.PFC_RANGING_REQUEST)
    {
      rangingRequestActionEnabled = true;
    }
    else
    {
      Serial.println("No mathcing command found");
    }
  }
  else if (msg != "" && !checksum_good)
  {
    Serial.println("Command with invalid checksum received: " + msg);
  }
}

void Actions::runSensorAction(Sensors &sensors)
{
  // Read all sensors
  sensors.readSensors();
}

void Actions::runGpsAction(Navigation &navigation)
{
  navigation.readGps(navigation.navigation_data);
}

void Actions::runLoggingAction(Logging &logging, Navigation &navigation, Sensors &sensors)
{
  // Log the data to the sd card
  String packet = createLoggablePacket(sensors, navigation);
  logging.writeTelemetry(packet);
}

void Actions::runPongAction(Communication &communication, Config &config)
{
  communication.sendRadio(config.PFC_PONG);
  pongActionEnabled = false;
}

void Actions::runStatusAction(Communication &communication, Config &config)
{
  // Create the status message
  String status = "PFC STATUS: NOT YET IMPLEMENTED";
  // Send the status message
  communication.sendRadio(status);
  statusActionEnabled = false;
}

void Actions::runMosfet1Action(Communication &communication, Config &config)
{
  communication.sendRadio("PFC MOSFET 1: NOT YET IMPLEMENTED");
  mosfet1ActionEnabled = false;
}

void Actions::runMosfet2Action(Communication &communication, Config &config)
{
  communication.sendRadio("PFC MOSFET 2: NOT YET IMPLEMENTED");
  mosfet2ActionEnabled = false;
}

void Actions::runDataRequestAction(Sensors &sensors, Navigation &navigation, Communication &communication, Config &config)
{
  runDataSendAction(sensors, navigation, communication, config);
  dataRequestActionEnabled = false;
}

void Actions::runRangingRequestAction(Navigation &navigation, Config &config)
{
  runRangingAction(navigation, config);
  rangingRequestActionEnabled = false;
}

void Actions::runDataSendAction(Sensors &sensors, Navigation &navigation, Communication &communication, Config &config)
{
  String packet = createSendablePacket(sensors, navigation);
  communication.msgToUkhas(packet, config);
  communication.sendRadio(packet);
}

void Actions::runRangingAction(Navigation &navigation, Config &config)
{
  navigation.readRanging(config, navigation.navigation_data);
}

String Actions::createSendablePacket(Sensors &sensors, Navigation &navigation)
{
  String packet = "";
  packet += String(sendable_packet_id);
  packet += ",";
  packet += String(navigation.navigation_data.gps.hour);
  packet += ":";
  packet += String(navigation.navigation_data.gps.minute);
  packet += ":";
  packet += String(navigation.navigation_data.gps.second);
  packet += ",";
  packet += String(navigation.navigation_data.gps.lat, 7);
  packet += ",";
  packet += String(navigation.navigation_data.gps.lng, 7);
  packet += ",";
  packet += String(navigation.navigation_data.gps.altitude, 2);
  packet += ",";
  packet += String(sensors.data.outsideThermistor.temperature, 2);
  packet += ",";
  packet += String(navigation.navigation_data.gps.satellites);
  packet += ",";
  packet += String(sensors.data.onBoardBaro.pressure);
  packet += ",";
  packet += String(navigation.navigation_data.gps.speed, 2);
  packet += ",";
  packet += String(sensors.data.onBoardBaro.altitude, 2);

  sendable_packet_id++;

  return packet;
}

String Actions::createLoggablePacket(Sensors &sensors, Navigation &navigation)
{
  String packet = "";
  packet += String(loggable_packed_id);
  packet += ",";
  packet += String(navigation.navigation_data.gps.hour);
  packet += ":";
  packet += String(navigation.navigation_data.gps.minute);
  packet += ":";
  packet += String(navigation.navigation_data.gps.second);
  packet += ",";
  packet += String(navigation.navigation_data.gps.lat, 7);
  packet += ",";
  packet += String(navigation.navigation_data.gps.lng, 7);
  packet += ",";
  packet += String(navigation.navigation_data.gps.altitude, 2);
  packet += ",";
  packet += String(sensors.data.outsideThermistor.temperature, 2);
  packet += ",";
  packet += String(navigation.navigation_data.gps.satellites);
  packet += ",";
  packet += String(sensors.data.onBoardBaro.pressure);
  packet += ",";
  packet += String(navigation.navigation_data.gps.speed, 2);
  packet += ",";
  packet += String(sensors.data.onBoardBaro.altitude, 2);

  loggable_packed_id++;

  return packet;
}