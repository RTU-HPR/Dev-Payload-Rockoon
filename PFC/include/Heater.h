#pragma once
#include <Config.h>

class Heater
{
private:  
  // State
  bool _heater_enabled = false;
  
  // Config
  Config::Heater_Config _heater_config; // Local copy of the config

  // Timing
  unsigned long int _last_pid_calculation_time = 0;
  unsigned long int _time_at_temperature_step_start = 0;

  // PID values
  float _proportional_term = 0;
  float _last_proportional_term = 0;
  float _derivative_term = 0;
  float _integral_term = 0;
  
  // Temperature values
  float _current_temperature_step = 0;
  float _heater_pwm = 0;

  // Function declarations
  void calculatePidValues(const float &container_temp);
  void calculateHeaterPwm();

  void setHeaterPwm();
  void reset();
  
public:
  // Function declarations
  void begin(const Config::Heater_Config &heater_config);
  void enableHeater(const float &container_temp);
  bool update(const float &container_temp);

  bool isHeaterEnabled() { return _heater_enabled; }
  void disableHeater() { _heater_enabled = false; }

  float getHeaterPwm() { return _heater_pwm; };
  void getPidValues(float &p, float &i, float &d) {p = _proportional_term; i = _integral_term; d = _derivative_term; }
  float getCurrentTemperatureStep() { return _current_temperature_step; }
  float getTargetTemperature() { return _heater_config.target_temp; }
};