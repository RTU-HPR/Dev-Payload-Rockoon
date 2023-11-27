#pragma once
#include <SPI.h>
#include <Wire.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

// PINS  Payload V2  (if not used just comment it out don't delete)
const int SERIAL1_RX = 1;
const int SERIAL1_TX = 0;
const long SERIAL1_BAUDRATE = 9600;

void start()
{
    Serial1.setRX(SERIAL1_RX);
    Serial1.setTX(SERIAL1_TX);
    Serial1.setFIFOSize(256); // once had a problem of not reading serial properly but this seemed to fix it
    Serial1.begin(SERIAL1_BAUDRATE);

    TinyGPSPlus _gps;
    SerialUART *_gps_serial = &Serial1;

    while (true)
    {
        while (_gps_serial->available() > 0)
        {
            _gps.encode(_gps_serial->read());

            if (_gps.location.isUpdated())
            {
                double new_gps_lat = _gps.location.lat();
                double new_gps_lng = _gps.location.lng();

                // SANITY CHECK, BECAUSE THERE IS NOTHING ELSE TO REALLY CHECK
                // Check if location is 0 (not yet established) or somewhere in the northern eastern Europe
                if ((new_gps_lat == 0 && new_gps_lng == 0) || ((50 <= new_gps_lat && new_gps_lat <= 60) && (15 <= new_gps_lng && new_gps_lng <= 35)))
                {
                    Serial.println("Lat: " + String(new_gps_lat, 6) + " lon: " + String(new_gps_lng, 6) + " height: " + String(_gps.altitude.meters(), 3) + " satellites: " + String(_gps.satellites.value()));
                    // data.gps_lat = new_gps_lat;
                    // data.gps_lng = new_gps_lng;
                    // data.gps_height = _gps.altitude.meters();
                    // data.gps_satellites = _gps.satellites.value();
                    // data.gps_time = _gps.time.value();
                }
                else
                {
                    Serial.println("Out of bounds");
                }
            }
        }
        delay(100);
    }
}
