#pragma once
#include <Wire.h> //Needed for I2C to GNSS

#include <SparkFun_u-blox_GNSS_Arduino_Library.h> //http://librarymanager/All#SparkFun_u-blox_GNSS

SFE_UBLOX_GNSS gps;

long lastTime = 0; //Simple local timer. Limits amount if I2C traffic to u-blox module.

int _SDA = 0;
int _SCL = 1;
TwoWire *_WIRE = &Wire;
int gps_address = 0x42;


void start()
{
  _WIRE->setSCL(_SCL);
  _WIRE->setSDA(_SDA);
  _WIRE->begin();

  Serial.println(F("SparkFun u-blox Example"));

  //myGNSS.enableDebugging(); // Uncomment this line to enable debug messages

  if (gps.begin(Wire, gps_address) == false) //Connect to the u-blox module using Wire port
  {
    Serial.println(F("u-blox GNSS not detected at default I2C address. Please check wiring. Freezing."));
    while (1)
      ;
  }

  gps.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)

  // If we are going to change the dynamic platform model, let's do it here.
  // Possible values are:
  // PORTABLE, STATIONARY, PEDESTRIAN, AUTOMOTIVE, SEA, AIRBORNE1g, AIRBORNE2g, AIRBORNE4g, WRIST, BIKE

  if (gps.setDynamicModel(DYN_MODEL_AIRBORNE2g) == false) // Set the dynamic model to PORTABLE
  {
    Serial.println(F("*** Warning: setDynamicModel failed ***"));
  }
  else
  {
    Serial.println(F("Dynamic platform model changed successfully!"));
  }

  // Let's read the new dynamic model to see if it worked
  uint8_t newDynamicModel = gps.getDynamicModel();
  if (newDynamicModel == DYN_MODEL_UNKNOWN)
  {
    Serial.println(F("*** Warning: getDynamicModel failed ***"));
  }
  else
  {
    Serial.print(F("The new dynamic model is: "));
    Serial.println(newDynamicModel);
  }

  //gps.saveConfigSelective(VAL_CFG_SUBSEC_NAVCONF); //Uncomment this line to save only the NAV settings to flash and BBR
  while (true)
  {
    //Query module only every second. Doing it more often will just cause I2C traffic.
    //The module only responds when a new position is available
    if (millis() - lastTime > 1000)
    {
      lastTime = millis(); //Update the timer

      long latitude = gps.getLatitude();
      Serial.print(F("Lat: "));
      Serial.print(latitude);

      long longitude = gps.getLongitude();
      Serial.print(F(" Long: "));
      Serial.print(longitude);
      Serial.print(F(" (degrees * 10^-7)"));

      long altitude = gps.getAltitude();
      Serial.print(F(" Alt: "));
      Serial.print(altitude);
      Serial.print(F(" (mm)"));

      Serial.println();
    }
  }
}