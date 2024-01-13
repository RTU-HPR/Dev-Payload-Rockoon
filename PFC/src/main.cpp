#include <Payload.h>

Payload payload;

// Performance monitoring
unsigned long lastLoopMillis = 0;
extern int loopTime; // Set as global variable
int loopTime = 0;

void setup()
{
  // INCREASES SPI CLOCK SPEED TO BE THE SAME AS CPU SPEED
  // MUST BE AT THE START OF SETUP
  // Get the processor sys_clk frequency in Hz
  uint32_t freq = clock_get_hz(clk_sys);

  // clk_peri does not have a divider, so input and output frequencies will be the same
  clock_configure(clk_peri,
                  0,
                  CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
                  freq,
                  freq);

  payload.begin();
  Serial.println("Payload setup complete");
  Serial.println("CPU Speed: " + String(rp2040.f_cpu() / 1000000) + " MHz");
  Serial.println();
}

void loop()
{
  lastLoopMillis = millis();
  payload.actions.runAllActions(payload.sensors, payload.navigation, payload.communication, payload.logging, payload.heater, payload.config);
  loopTime = millis() - lastLoopMillis;
}
