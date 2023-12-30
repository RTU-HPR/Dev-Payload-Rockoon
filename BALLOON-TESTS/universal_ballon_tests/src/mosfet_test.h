#pragma once
// PINS   (if not used just comment it out don't delete)
const int heater_mosfet = 16;
const int recovery_mosfet_1 = 18;
const int recovery_mosfet_2 = 19;

void start()
{
  pinMode(recovery_mosfet_1, OUTPUT_12MA);
  pinMode(recovery_mosfet_2, OUTPUT_12MA);
  pinMode(heater_mosfet, OUTPUT_12MA);
    while (true)
    {
      digitalWrite(recovery_mosfet_1, HIGH);
      digitalWrite(recovery_mosfet_2, HIGH);
      digitalWrite(heater_mosfet, HIGH);
      delay(3000);
      digitalWrite(recovery_mosfet_1, LOW);
      digitalWrite(recovery_mosfet_2, LOW);
      digitalWrite(heater_mosfet, LOW);
      delay(3000);
    }
}
