#include "Payload.h"

bool Payload::initCommunicationBusses()
{
  bool success = true;
  
  // Wire0
  if (Wire.setSCL(config.WIRE0_SCL) && Wire.setSDA(config.WIRE0_SDA))
  {
    Wire.begin();
    Serial.println("Wire0 communication bus initialized");
  }
  else
  {
    
    errorString =+ "Wire0 begin fail | ";
    success = false;
  }

  // Wire1
  if (Wire1.setSCL(config.WIRE1_SCL) && Wire1.setSDA(config.WIRE1_SDA))
  {
    Wire1.begin();
    Serial.println("Wire1 communication bus initialized");
  }
  else
  {
    errorString =+ "Wire1 begin fail | ";
    success = false;
  }

  // SPI
  if (SPI.setRX(config.SPI0_RX) && SPI.setTX(config.SPI0_TX) && SPI.setSCK(config.SPI0_SCK))
  {
    SPI.begin();
    Serial.println("SPI0 communication bus initialized");
  }
  else
  {
    errorString =+ "SPI0 begin fail | ";
    success = false;
  }

  return success;
}

void Payload::begin()
{
  // Initialize PC serial
  Serial.begin(config.PC_BAUDRATE);
  if (config.WAIT_PC)
  {
    while (!Serial)
    {
      delay(100);
    }
  }

  Serial.println("PC Serial initialized");

  // Initialize the communication busses
  if (initCommunicationBusses())
  {
    Serial.println("All communication busses initialized successfully");
  }
  else
  {
    Serial.println("Error in initializing communication busses");
  }

  // Enable sensor power
  pinMode(config.SENSOR_POWER_ENABLE_PIN, OUTPUT_12MA);
  digitalWrite(config.SENSOR_POWER_ENABLE_PIN, HIGH);
  Serial.println("Sensor power enabled");

  // Initialize the SD card
  if (!logging.begin(config))
  {
    errorString =+ "SD begin fail | ";
  }
  else
  {
    Serial.println("SD card initialized successfully");

    // Read config file
    if (!logging.readConfig(config))
    {
      errorString =+ "New config file created | ";
    }
    else
    {
      Serial.println("Config file read successfully");
    }
  }

  // Initialise the radio
  if (!communication.beginRadio(config))
  {
    errorString =+ "Radio begin fail | ";
  }
  else
  {
    Serial.println("Radio initialized successfully");
  }

  Serial.println();
  
  // Send inital error string
  if (errorString != "")
  {
    Serial.println("INITAL ERRORS: " + errorString);
    Serial.println();
    logging.writeError(errorString);
    communication.sendError(errorString);
  }

  // Initialise all sensors
  if (!sensors.begin(config))
  {
    Serial.println("Error initializing sensors");
  }
  else
  {
    Serial.println("Sensors initialized successfully");
  }

  // Send inital error string
  if (sensors.sensorErrorString != "")
  {
    Serial.println("SENSOR ERRORS: " + errorString);
    Serial.println();
    logging.writeError(errorString);
    communication.sendError(errorString);
  }

  // Initialise GPS
  if (!navigation.beginGps(config.gps_config))
  {
    String error = "GPS begin fail";
    Serial.println(error);
    logging.writeError(error);
    communication.sendError(error);
  }
  else
  {
    Serial.println("GPS initialized successfully");
  }

  // Initialise ranging
  if (!navigation.beginRanging(config.ranging_device, config.ranging_mode))
  {
    String error = "Ranging begin fail";
    Serial.println(error);
    logging.writeError(error);
    communication.sendError(error);
  }
  else
  {
    Serial.println("Navigation initialized successfully");
  }
  
  Serial.println();
}