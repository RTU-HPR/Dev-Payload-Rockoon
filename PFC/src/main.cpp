#include <Payload.h>

Payload payload;

unsigned long last_time = 0;

void setup()
{
  payload.begin();
  Serial.println("Payload setup complete");
  Serial.println();
}

void loop()
{
  // Read all sensors
  payload.sensors.readSensors();

  if (millis() - last_time > 500)
  {
    Serial.println("MS56XX data: " + String(payload.sensors.data.onBoardBaro.pressure) + " Pa, " + String(payload.sensors.data.onBoardBaro.temperature) + " C");

    Serial.println("IMU data: " + String(payload.sensors.data.imu.accel.acceleration.x) + " m/s^2, " + String(payload.sensors.data.imu.accel.acceleration.y) + " m/s^2, " + String(payload.sensors.data.imu.accel.acceleration.z) + " m/s^2");

    Serial.println("Battery voltage: " + String(payload.sensors.data.battery.voltage) + " V");

    Serial.println("Outside thermistor: " + String(payload.sensors.data.outsideThermistor.temperature) + " C");

    Serial.println("Container barometer: " + String(payload.sensors.data.containerBaro.pressure) + " Pa, " + String(payload.sensors.data.containerBaro.temperature) + " C");

    Serial.println("Container temperature: " + String(payload.sensors.data.containerTemperature.temperature) + " C");

    Serial.println();

    last_time = millis();
  }
}
