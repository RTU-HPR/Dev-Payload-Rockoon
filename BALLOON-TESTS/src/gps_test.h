#pragma once
#include <SPI.h>
#include <Wire.h>
#include <TinyGPS++.h>
#include <PCF8575.h>

// PINS  Payload V2  (if not used just comment it out don't delete)
const int SERIAL1_RX = 1;
const int SERIAL1_TX = 0;
const long SERIAL1_BAUDRATE = 38400; // Default gps module baud rate


void start()
{
    Serial1.setRX(SERIAL1_RX);
    Serial1.setTX(SERIAL1_TX);
    Serial1.setFIFOSize(256); // once had a problem of not reading serial properly but this seemed to fix it
    Serial1.begin(SERIAL1_BAUDRATE);

    TinyGPSPlus _gps;
    SerialUART *_gps_serial = &Serial1;
    
    // Command for setting gps to airborne <1g mode
    const static char PROGMEM airborne[] = {0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4D, 0xDB};

    Serial1.write(airborne, sizeof(airborne));
    delay(100);
    
    while (true)
    {
        while (_gps_serial->available() > 0)
        {
            _gps.encode(_gps_serial->read());
            //Serial.print((char)_gps_serial->read());

            if (_gps.location.isUpdated())
            {
                doubt le new_gps_lat = _gps.location.lat();
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
