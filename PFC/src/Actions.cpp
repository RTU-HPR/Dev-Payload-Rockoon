#include <Actions.h>
#include <Config.h>

// unsigned long last_time_1 = 0;
// unsigned long last_time_2 = 0;

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
    runRequestedActions(sensors, navigation, communication, logging, config);
    // Serial.println("Requested actions time: " + String(millis() - last_time_1) + "ms");
}

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

void Actions::runTimedActions(Sensors &sensors, Navigation &navigation, Communication &communication, Config &config)
{
    // Only start timed actions 10 seconds after turning on for safety purposes
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

void Actions::runRequestedActions(Sensors &sensors, Navigation &navigation, Communication &communication, Logging &logging, Config &config)
{
    if (!requestedActionEnabled)
    {
        return;
    }
    if (millis() - lastCommunicationCycle >= config.COMMUNICATION_RESPONSE_SEND_TIME && millis() - lastCommunicationCycle <= config.COMMUNICATION_ESSENTIAL_DATA_SEND_TIME)
    {
    }
    else
    {
        return;
    }

    if (infoErrorRequestActionEnabled)
    {
        runInfoErrorSendAction(communication, logging, config);
    }
    if (completeDataRequestActionEnabled)
    {
        runCompleteDataRequestAction(sensors, navigation, communication, config);
    }
    if (formatStorageActionEnabled)
    {
        runFormatStorageAction(communication, logging, config);
    }
    if (heaterSetActionEnabled)
    {
        runHeaterSetAction(communication, config);
    }
    if (pyroFireActionEnabled)
    {
        runPyroFireAction(communication, config);
    }
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
            // get BS time epoch
            String values[2];
            logging.parseString(msg, values, 2);
        }
        else if (msg.substring(0, config.PFC_COMPLETE_DATA_REQUEST.length()) == config.PFC_COMPLETE_DATA_REQUEST)
        {
            completeDataRequestActionEnabled = true;
            // get BS time epoch
            String values[2];
            logging.parseString(msg, values, 2);
        }
        else if (msg.substring(0, config.PFC_FORMAT_REQUEST.length()) == config.PFC_FORMAT_REQUEST)
        {
            formatStorageActionEnabled = true;
            // get BS time epoch
            String values[2];
            logging.parseString(msg, values, 2);
        }
        else if (msg.substring(0, config.PFC_HEATER_REQUEST.length()) == config.PFC_HEATER_REQUEST)
        {
            heaterSetActionEnabled = true;
            String values[3];
            logging.parseString(msg, values, 3);
            // need to add setting of epoch
            heaterState = (bool)values[3].toInt();
        }
        else if (msg.substring(0, config.PFC_PYRO_REQUEST.length()) == config.PFC_PYRO_REQUEST)
        {
            pyroFireActionEnabled = true;
            // get BS time epoch
            // get pyro channel
            String values[3];
            logging.parseString(msg, values, 3);
            // need to add setting of epoch
            pyroChannel = values[3].toInt();
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

void Actions::runEssentialDataSendAction(Sensors &sensors, Navigation &navigation, Communication &communication, Config &config)
{
    if (millis() - lastCommunicationCycle >= config.COMMUNICATION_ESSENTIAL_DATA_SEND_TIME && millis() - lastCommunicationCycle <= config.COMMUNICATION_CYCLE_INTERVAL)
    {
        // need to implement completeDataMSG proper sensor values
        String essentialDataMSG = "1.00,2.00,3.00,4.00,5,6,1,0";
        String msg = config.PFC_ESSENTIAL_DATA_RESPONSE + "," + dataEssentialResponseId + "," + essentialDataMSG;
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

// Timed and Requested actions
void Actions::runInfoErrorSendAction(Communication &communication, Logging &logging, Config &config)
{
    String infoErrorMSG = "!Sample error";
    String msg = config.PFC_INFO_ERROR_RESPONSE + "," + infoErrorResponseId + "," + infoErrorMSG;
    communication.msgToUkhas(msg, config);
    if (!communication.sendRadio(msg))
    {
        Serial.println("Info error response sent");
        return;
    }
    infoErrorResponseId++;
    infoErrorRequestActionEnabled = false;
    requestedActionEnabled = false;
}

void Actions::runCompleteDataRequestAction(Sensors &sensors, Navigation &navigation, Communication &communication, Config &config)
{
    // need to implement completeDataMSG proper sensor values
    String completeDataMSG = "1.00,2.00,3.00,4.00, 255,5.00,6.00,420,421,7.00";
    String msg = config.PFC_COMPLETE_DATA_RESPONSE + "," + completeDataResponseId + "," + completeDataMSG;
    communication.msgToUkhas(msg, config);
    if (!communication.sendRadio(msg))
    {
        Serial.println("Complete data response sent");
        return;
    }
    completeDataResponseId++;
    completeDataRequestActionEnabled = false;
    requestedActionEnabled = false;
}

void Actions::runFormatStorageAction(Communication &communication, Logging &logging, Config &config)
{
    // need to implement formatting of SD card

    String msg = config.PFC_FORMAT_RESPONSE + "," + formatResponseId + "," + "1";
    communication.msgToUkhas(msg, config);
    if (!communication.sendRadio(msg))
    {
        Serial.println("Format response sent");
        return;
    }
    formatResponseId++;
    formatStorageActionEnabled = false;
    requestedActionEnabled = false;
}

void Actions::runHeaterSetAction(Communication &communication, Config &config)
{
    // need to implement heater mode setting
    String msg = config.PFC_HEATER_RESPONSE + "," + heaterResponseId + "," + "1";
    communication.msgToUkhas(msg, config);
    if (!communication.sendRadio(msg))
    {
        Serial.println("Heater response sent");
        return;
    }
    heaterResponseId++;
    heaterSetActionEnabled = false;
    requestedActionEnabled = false;
}

void Actions::runPyroFireAction(Communication &communication, Config &config)
{
    // need to implement pyro firing

    String msg = config.PFC_PYRO_RESPONSE + "," + pyroResponseId + "," + "1";
    communication.msgToUkhas(msg, config);
    if (!communication.sendRadio(msg))
    {
        Serial.println("Pyro response sent");
        return;
    }
    pyroResponseId++;
    infoErrorRequestActionEnabled = false;
    requestedActionEnabled = false;
}

// void Actions::runStatusAction(Sensors &sensors, Navigation &navigation, Communication &communication, Config &config)
// {
//     // Create the status message
//     String status = createStatusPacket(sensors, navigation, config);
//     // Send the status message
//     communication.sendRadio(status);
//     statusActionEnabled = false;
// }

// void Actions::runMosfet1Action(Communication &communication, Config &config)
// {
//     communication.sendRadio("PFC MOSFET 1: NOT YET IMPLEMENTED");
//     mosfet1ActionEnabled = false;
// }

// void Actions::runMosfet2Action(Communication &communication, Config &config)
// {
//     communication.sendRadio("PFC MOSFET 2: NOT YET IMPLEMENTED");
//     mosfet2ActionEnabled = false;
// }

// void Actions::runDataRequestAction(Sensors &sensors, Navigation &navigation, Communication &communication, Config &config)
// {
//     runDataSendAction(sensors, navigation, communication, config);
//     dataRequestActionEnabled = false;
// }

// void Actions::runRangingRequestAction(Navigation &navigation, Config &config)
// {
//     runRangingAction(navigation, config);
//     rangingRequestActionEnabled = false;
// }

// void Actions::runDataSendAction(Sensors &sensors, Navigation &navigation, Communication &communication, Config &config)
// {
//     String packet = createSendablePacket(sensors, navigation);
//     communication.msgToUkhas(packet, config);
//     communication.sendRadio(packet);
// }

void Actions::runRangingAction(Navigation &navigation, Config &config)
{
    navigation.readRanging(config, navigation.navigation_data);
}

void Actions::runGetCommunicationCycleStartAction(Navigation &navigation, Config &config)
{
    if (millis() - lastCommunicationCycle <= 3000)
    {
        // Serial.println("Communication cycle already started: " + String(millis() - lastCommunicationCycle));
        return;
    }
    if (navigation.navigation_data.gps.epoch_time == 0)
    {
        Serial.println("GPS epoch time: " + String(navigation.navigation_data.gps.epoch_time));
        return;
    }

    int comm_cycle_interval_sec = config.COMMUNICATION_CYCLE_INTERVAL / 1000;
    if (navigation.navigation_data.gps.epoch_time % comm_cycle_interval_sec == 0 || navigation.navigation_data.gps.epoch_time % comm_cycle_interval_sec + 1 == 0 || navigation.navigation_data.gps.epoch_time % comm_cycle_interval_sec - 1 == 0)
    {
        lastCommunicationCycle = millis();
        dataEssentialSendActionEnabled = true;
        requestedActionEnabled = true;
        Serial.println("New communication cycle started: " + String(lastCommunicationCycle) + " " + String(navigation.navigation_data.gps.epoch_time));
    }
}

String Actions::createStatusPacket(Sensors &sensors, Navigation &navigation, Config &config)
{
    String packet = "";
    // packet += String(config.PFC_STATUS_SEND);
    packet += ",";
    packet += String(sendable_packet_id);
    packet += ",";
    packet += String(navigation.navigation_data.gps.hour);
    packet += ":";
    packet += String(navigation.navigation_data.gps.minute);
    packet += ":";
    packet += String(navigation.navigation_data.gps.second);
    packet += ",";
    packet += String(millis() / 1000);
    return packet;
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
    packet += ",";
    packet += String(millis() / 1000);
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
    packet += ",";
    packet += String(loopTime);

    loggable_packed_id++;

    return packet;
}