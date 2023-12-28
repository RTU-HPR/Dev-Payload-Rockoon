#include "Payload.h"

bool Payload::initCommunicationBusses()
{
  // Wire0
  if (Wire.setSCL(config.WIRE0_SCL) && Wire.setSDA(config.WIRE0_SDA))
  {
    Wire.begin();
    Serial.println("Wire0 communication bus initialized");
  }
  else
  {
    Serial.println("Wire0 communication bus failed to initialize");
    return false;
  }

  // Wire1
  if (Wire1.setSCL(config.WIRE1_SCL) && Wire1.setSDA(config.WIRE1_SDA))
  {
    Wire1.begin();
    Serial.println("Wire1 communication bus initialized");
  }
  else
  {
    Serial.println("Wire1 communication bus failed to initialize");
    return false;
  }

  // SPI
  if (SPI.setRX(config.SPI0_RX) && SPI.setTX(config.SPI0_TX) && SPI.setSCK(config.SPI0_SCK))
  {
    SPI.begin();
    Serial.println("SPI0 communication bus initialized");
  }
  else
  {
    Serial.println("SPI0 communication bus failed to initialize");
    return false;
  }

  // All communication busses were successfully initialised
  return true;
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
    Serial.println("Error initializing communication busses");
  }

  // Initialize the SD card
  if (!logging.begin(config))
  {
    Serial.println("Error initializing SD card");
  }
  else
  {
    Serial.println("SD card initialized successfully");
  }

  // Enable sensor power
  pinMode(config.SENSOR_POWER_ENABLE_PIN, OUTPUT_12MA);
  digitalWrite(config.SENSOR_POWER_ENABLE_PIN, HIGH);
  Serial.println("Sensor power enabled");

  // Initialise the radio
  if (!communication.beginRadio(config))
  {
    // If the radio fails to initialize, stop further execution
    while (true)
    {
      Serial.println("Error initializing radio");
      delay(1000);
    }
  }
  else
  {
    Serial.println("Radio initialized successfully");
  }

  // // Initialise the navigation
  // if (!navigation.beginGps(config.gps_config))
  // {
  //   Serial.println("Error initializing GPS");
  // }
  // else
  // {
  //   Serial.println("Navigation initialized successfully");
  // }

  // if (!navigation.beginRanging(config.ranging_device, config.ranging_mode))
  // {
  //   Serial.println("Error initializing ranging");
  // }
  // else
  // {
  //   Serial.println("Navigation initialized successfully");
  // }

  // Initialise the sensors
  if (!sensors.begin(config))
  {
    Serial.println("Error initializing sensors");
  }
  else
  {
    Serial.println("Sensors initialized successfully");
  }
}