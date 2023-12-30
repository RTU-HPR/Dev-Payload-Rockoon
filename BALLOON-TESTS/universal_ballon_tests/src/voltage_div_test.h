#pragma once
// PINS   (if not used just comment it out don't delete)
const int BATT_SENS_PIN = 26;
const int HEATER_CURR_SENS_PIN = 27;
const float CONVERSION_FACTOR =  3.3 * 3.1251;

void start()
{
  analogReadResolution(12);
  pinMode(BATT_SENS_PIN, INPUT);
  pinMode(HEATER_CURR_SENS_PIN, INPUT);
  
  pinMode(16, OUTPUT_12MA);
  digitalWrite(16, LOW);
  while (true)
  {
    // Read voltage and do calculations
    float adc_reading = analogRead(BATT_SENS_PIN);
    float new_batt_voltage = (adc_reading / 4095.0) * CONVERSION_FACTOR;

    Serial.print("Battery voltage: " + String(new_batt_voltage, 3) + " Volts | ");
  
    // Read voltage and do calculations
    adc_reading = analogRead(HEATER_CURR_SENS_PIN);
    Serial.print("Heater analog value: " +String(adc_reading) + " | ");
    float voltage = (adc_reading / 4095.0) * CONVERSION_FACTOR;

    // Using Ohm's law calculate current (I=U/R)
    float new_current = voltage / 1;

    Serial.println("Heater current: " + String(voltage, 4) + " Amps");
    delay(100);
  }
}
