#pragma once
// PINS   (if not used just comment it out don't delete)

const int switch_pin = 21;

void start()
{
  pinMode(switch_pin, INPUT);
  while (true)
  {
    Serial.println("Switch state: " + String(digitalRead(switch_pin)));
    delay(10);
  }
}
