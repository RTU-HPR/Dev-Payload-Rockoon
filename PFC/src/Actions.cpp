#include <Actions.h>
#include <Config.h>

//unsigned long last_time_1 = 0;
//unsigned long last_time_2 = 0;

void Actions::runAllActions(Sensors &sensors, Navigation &navigation, Communication &communication, Logging &logging, Config &config)
{
  // last_time_1 = millis();
  // Receive commands, read sensors and gps, log data to sd card
  runContinousActions(sensors, navigation, communication, logging, config);
  // Serial.println("Continous actions time: " + String(millis() - last_time_1) + "ms");
  // Do ranging and send telemetry data
  // last_time_1 = millis();
  runTimedActions(sensors, navigation, communication, config);
  // Serial.println("Timed actions time: " + String(millis() - last_time_1) + "ms");

  // Do actions requested by a command
  // last_time_1 = millis();
  runRequestedActions(sensors, navigation, communication, config);
  // Serial.println("Requested actions time: " + String(millis() - last_time_1) + "ms");

}

void Actions::runContinousActions(Sensors &sensors, Navigation &navigation, Communication &communication, Logging &logging, Config &config)
{
  // last_time_2 = millis();
  // Run the command receive action
  if (commandReceiveActionEnabled)
  {
    runCommandReceiveAction(communication, config);
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
  // Serial.println("GPS action time: " + String(millis() - last_time_2) + "ms");
  // last_time_2 = millis();
  // Run the logging action
  if (loggingActionEnabled)
  {
    runLoggingAction(logging, navigation, sensors);
  }
  // Serial.println("Logging action time: " + String(millis() - last_time_2) + "ms");
}

void Actions::runTimedActions(Sensors &sensors, Navigation &navigation, Communication &communication, Config &config)
{
  // Only start timed actions 10 seconds after turning on for safety purposes
  if (millis() < config.TIMED_ACTION_INITIAL_DELAY)
  {
    return;
  }

  // Run the ranging the set time before data send action
  if (rangingSendActionEnabled)
  {
    if (millis() - lastRangingActionMillis >= config.RANGING_ACTION_SAFETY_INTERVAL)
    {
      if (millis() - lastDataSendActionMillis >= config.DATA_SEND_ACTION_INTERVAL - config.RANGING_ACTION_PREEMPTIVE_INTERVAL)
      {
        runRangingAction(navigation, config);
        lastRangingActionMillis = millis();
      }
    }
  }

  // Using gps time and millis for redundancy purposes
  // Run the data send action, if it is enabled, GPS secoconds are dividiable by 15 or atleast 15,5 seconds have passed since the last transmit
  if (dataSendActionEnabled)
  {
    bool shouldSend = false;
    if (navigation.navigation_data.gps.epoch_time != 0)
    {
      Serial.println("Epoch time is not 0");
      if (navigation.navigation_data.gps.second % 15 == 0 || millis() - lastDataSendActionMillis >= config.DATA_SEND_ACTION_INTERVAL + 500)
      {
        Serial.println("Seconds are dividiable by 15 or atleast 15,5 seconds have passed since the last transmit");
        Serial.println("Seconds: " + String(navigation.navigation_data.gps.second));
        Serial.println("Millis: " + String(millis() - lastDataSendActionMillis));
        shouldSend = true;
      }
    }
    else if (millis() - lastDataSendActionMillis >= config.DATA_SEND_ACTION_INTERVAL + 500)
    {
      Serial.println("Time since last data send action is greater than the interval + 500ms");
      Serial.println("Millis: " + String(millis() - lastDataSendActionMillis));
      shouldSend = true;
    }
    if (shouldSend)
    {
      Serial.println("Sending data");
      runDataSendAction(sensors, navigation, communication, config);
      lastDataSendActionMillis = millis();
    }
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

  
  loggable_packed_id++;

  return packet;
}