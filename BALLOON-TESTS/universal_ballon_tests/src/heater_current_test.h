#pragma once
// PINS   (if not used just comment it out don't delete)
const int heater_mosfet = 16;

void start()
{
  pinMode(heater_mosfet, OUTPUT);
  analogWriteRange(1000);
  analogWriteFreq(100);
  analogReadResolution(12);
  int i = 0;
  float duty_cycle = 0;
  while (true)
  {
    while (Serial.available() > 0)
    {
      String msg = Serial.readString();
      msg.trim();
      i = msg.toInt();
    }
    analogWrite(heater_mosfet, i);
    unsigned long time = millis();
    while (millis() - time < 500)
    {
      float duty = i / 10.0;
      Serial.println("PWM: " + String(i) + " | Duty cycle: " + String(duty) + "%");
      delay(10);
    }
  }
}
