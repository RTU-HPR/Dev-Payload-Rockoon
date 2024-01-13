#include <Actions.h>

void Actions::runContinousActions(Sensors &sensors, Navigation &navigation, Communication &communication, Logging &logging, Config &config)
{
  // last_time_2 = millis();
  // Run the command receive action
  if (commandReceiveActionEnabled)
  {
    runCommandReceiveAction(communication, logging, config);
  }
  // Serial.println("Command receive action time: " + String(millis() - last_time_2) + "ms");
  // last_time_2 = millis();
  // Run the sensor action
  if (sensorActionEnabled)
  {
    runSensorAction(sensors);
  }
  // Serial.println("Sensor action time: " + String(millis() - last_time_2) + "ms");
  // last_time_2 = millis();
  // Run the navigation action
  if (gpsActionEnabled)
  {
    runGpsAction(navigation);
  }
  // Run the ranging the set time before data send action
  if (rangingSendActionEnabled)
  {
    runRangingAction(navigation, config);
  }
  // Check if the communication cycle should be started
  if (getCommunicationCycleStartActionEnabled)
  {
    runGetCommunicationCycleStartAction(navigation, config);
  }
  // Serial.println("GPS action time: " + String(millis() - last_time_2) + "ms");
  // last_time_2 = millis();
  // Run the logging action
  if (loggingActionEnabled)
  {
    runLoggingAction(logging, navigation, sensors);
  }
  // Serial.println("Logging action time: " + String(millis() - last_time_2) + "ms");
}

void Actions::runCommandReceiveAction(Communication &communication, Logging &logging, Config &config)
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
    if (msg.substring(0, config.PFC_INFO_ERROR_REQUEST.length()) == config.PFC_INFO_ERROR_REQUEST)
    {
      infoErrorRequestActionEnabled = true;
      String values[2];
      logging.parseString(msg, values, 2);
    }
    else if (msg.substring(0, config.PFC_COMPLETE_DATA_REQUEST.length()) == config.PFC_COMPLETE_DATA_REQUEST)
    {
      completeDataRequestActionEnabled = true;
      String values[2];
      logging.parseString(msg, values, 2);
    }
    else if (msg.substring(0, config.PFC_FORMAT_REQUEST.length()) == config.PFC_FORMAT_REQUEST)
    {
      formatStorageActionEnabled = true;
      String values[2];
      logging.parseString(msg, values, 2);
    }
    else if (msg.substring(0, config.PFC_HEATER_REQUEST.length()) == config.PFC_HEATER_REQUEST)
    {
      heaterSetActionEnabled = true;
      String values[3];
      logging.parseString(msg, values, 3);
      heaterState = (bool)values[3].toInt();
    }
    else if (msg.substring(0, config.PFC_PYRO_REQUEST.length()) == config.PFC_PYRO_REQUEST)
    {
      pyroFireActionEnabled = true;
      // get pyro channel
      String values[3];
      logging.parseString(msg, values, 3);
      int pyroChannel = values[3].toInt();
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

void Actions::runRangingAction(Navigation &navigation, Config &config)
{
  navigation.readRanging(config, navigation.navigation_data);
}

void Actions::runGetCommunicationCycleStartAction(Navigation &navigation, Config &config)
{
  // Serial.println("GPS epoch time: " + String(navigation.navigation_data.gps.epoch_time));
  if (millis() - lastCommunicationCycle <= 3000)
  {
    // Serial.println("Communication cycle already started: " + String(millis() - lastCommunicationCycle));
    return;
  }
  if (navigation.navigation_data.gps.epoch_time == 0)
  {
    return;
  }

  int comm_cycle_interval_sec = config.COMMUNICATION_CYCLE_INTERVAL / 1000;
  if (navigation.navigation_data.gps.second % comm_cycle_interval_sec == 0 || navigation.navigation_data.gps.second % comm_cycle_interval_sec + 1 == 0 || navigation.navigation_data.gps.second % comm_cycle_interval_sec - 1 == 0)
  {
    lastCommunicationCycle = millis();
    dataEssentialSendActionEnabled = true;
    requestedActionEnabled = true;
    Serial.println("New communication cycle started: " + String(lastCommunicationCycle));
  }
}

String Actions::createLoggablePacket(Sensors &sensors, Navigation &navigation)
{
    String packet = "";
    // UKHAS
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
    // CUSTOM
    // IMU
    packet += ",";
    packet += String(sensors.data.imu.accel.acceleration.x, 4);
    packet += ",";
    packet += String(sensors.data.imu.accel.acceleration.y, 4);
    packet += ",";
    packet += String(sensors.data.imu.accel.acceleration.z, 4);
    packet += ",";
    packet += String(sensors.data.imu.accel.acceleration.heading, 3);
    packet += ",";
    packet += String(sensors.data.imu.accel.acceleration.pitch, 3);
    packet += ",";
    packet += String(sensors.data.imu.accel.acceleration.roll, 3);
    packet += ",";
    packet += String(sensors.data.imu.gyro.gyro.x, 4);
    packet += ",";
    packet += String(sensors.data.imu.gyro.gyro.y, 4);
    packet += ",";
    packet += String(sensors.data.imu.gyro.gyro.z, 4);
    packet += ",";
    packet += String(sensors.data.imu.temp.temperature, 2);
    // MS56XX
    packet += ",";
    packet += String(sensors.data.onBoardBaro.temperature, 2);
    // Container temperature
    packet += ",";
    packet += String(sensors.data.containerTemperature.temperature, 2);
    // Container baro
    packet += ",";
    packet += String(sensors.data.containerBaro.temperature, 2);
    packet += ",";
    packet += String(sensors.data.containerBaro.pressure);
    packet += ",";
    // Battery
    packet += String(sensors.data.battery.voltage, 2);
    packet += ",";
    // GPS
    packet += String(navigation.navigation_data.gps.epoch_time);
    packet += ",";
    packet += String(navigation.navigation_data.gps.heading);
    packet += ",";
    packet += String(navigation.navigation_data.gps.pdop);
    // Ranging
    packet += ",";
    packet += String(navigation.navigation_data.ranging[0].distance, 2);
    packet += ",";
    packet += String(navigation.navigation_data.ranging[0].f_error, 2);
    packet += ",";
    packet += String(navigation.navigation_data.ranging[0].rssi, 2);
    packet += ",";
    packet += String(navigation.navigation_data.ranging[0].snr, 2);
    packet += ",";
    packet += String(navigation.navigation_data.ranging[0].time, 2);
    packet += ",";
    packet += String(navigation.navigation_data.ranging[1].distance, 2);
    packet += ",";
    packet += String(navigation.navigation_data.ranging[1].f_error, 2);
    packet += ",";
    packet += String(navigation.navigation_data.ranging[1].rssi, 2);
    packet += ",";
    packet += String(navigation.navigation_data.ranging[1].snr, 2);
    packet += ",";
    packet += String(navigation.navigation_data.ranging[1].time, 2);
    packet += ",";
    packet += String(navigation.navigation_data.ranging[2].distance, 2);
    packet += ",";
    packet += String(navigation.navigation_data.ranging[2].f_error, 2);
    packet += ",";
    packet += String(navigation.navigation_data.ranging[2].rssi, 2);
    packet += ",";
    packet += String(navigation.navigation_data.ranging[2].snr, 2);
    packet += ",";
    packet += String(navigation.navigation_data.ranging[2].time, 2);
    packet += ",";
    packet += String(navigation.navigation_data.ranging_position.lat, 7);
    packet += ",";
    packet += String(navigation.navigation_data.ranging_position.lng, 7);
    packet += ",";
    packet += String(navigation.navigation_data.ranging_position.height, 2);
    packet += ",";
    // MISC
    packet += String(millis());
    packet += ",";
    packet += String(rp2040.getUsedHeap());
    packet += ",";
    packet += String(loopTime);

    loggable_packed_id++;

    return packet;
}