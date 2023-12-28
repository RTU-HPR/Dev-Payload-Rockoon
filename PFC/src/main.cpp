#include <Payload.h>

Payload payload;

void setup()
{
  payload.begin();
  Serial.println("Payload setup complete");
  Serial.println();
}

void loop()
{
  payload.actions.runAllActions(payload.sensors, payload.navigation, payload.communication, payload.logging);
}
