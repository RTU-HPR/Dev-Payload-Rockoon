#include <Actions.h>
#include <Continous_Actions.h>
#include <Timed_Actions.h>
#include <Requested_Actions.h>

// unsigned long last_time_1 = 0;
// unsigned long last_time_2 = 0;

void Actions::runAllActions(Sensors &sensors, Navigation &navigation, Communication &communication, Logging &logging, Heater &heater, Config &config)
{
    // last_time_1 = millis();
    // Receive commands, read sensors and gps, log data to sd card
    runContinousActions(sensors, navigation, communication, logging, heater, config);
    // Serial.println("Continous actions time: " + String(millis() - last_time_1) + "ms");
    // Do ranging and send telemetry data
    // last_time_1 = millis();
    runTimedActions(sensors, navigation, communication, logging, config);
    // Serial.println("Timed actions time: " + String(millis() - last_time_1) + "ms");

    // Do actions requested by a command
    // last_time_1 = millis();
    runRequestedActions(sensors, navigation, communication, logging, heater, config);
    // Serial.println("Requested actions time: " + String(millis() - last_time_1) + "ms");
}
