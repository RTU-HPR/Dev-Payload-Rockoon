#include <Heater.h>

void Heater::begin(const Config::Heater_Config &heater_config)
{
  // Save the config
  _heater_config = heater_config;

  // Reset all values to default
  reset();

  // By default, the heater is disabled
  _heater_enabled = false;
}

void Heater::enableHeater(const float &container_temp)
{
  // Reset all values to default
  reset();

  // Set the current temperature step to the container temperature
  _current_temperature_step = constrain(ceil(container_temp), 0, _heater_config.target_temp);

  // Enable the heater
  _heater_enabled = true;
}

bool Heater::update(const float &container_temp)
{
  // If the heater is enabled, calculate the PID values and set the heater PWM
  if (!_heater_enabled)
  {
    return false;
  }
  
  calculatePidValues(container_temp);
  calculateHeaterPwm();
  setHeaterPwm();
  return true;
}

void Heater::calculatePidValues(const float &container_temp)
{
  // Time since last pid update
  float dt = (millis() - _last_pid_calculation_time);

  // If the temperature is 0.05 degree below or above the safe temp, reset the time at temp step
  if ((_current_temperature_step - 0.05) > container_temp || container_temp > (_current_temperature_step + 0.05))
  {
    _time_at_temperature_step_start = millis();
  }

  // Calculate proportional term based on the state
  if (_heater_config.target_temp == _current_temperature_step)
  {
    _proportional_term = _heater_config.target_temp - container_temp;
  }
  else
  {
    // If the temperature has stayed at the safe temp for 20 seconds, increase the safe temperature
    if ((millis() - _time_at_temperature_step_start) > 10000)
    {
      _current_temperature_step += 1;
      _time_at_temperature_step_start = millis();
    }
    _proportional_term = _current_temperature_step - container_temp;
  }

  // Integral term
  // Don't allow integral term to be negative
  _integral_term += _proportional_term * dt;

  // Derivative term
  // The derivative term might not actually be required, but it can be calculated if needed
  // _derivative_term = (_proportional_term - _last_proportional_term) / dt;
  _derivative_term = 0;

  // Make sure the the PID values are within the limits
  _proportional_term = constrain(_proportional_term, -_heater_config.Kp_limit, _heater_config.Kp_limit);
  _integral_term = constrain(_integral_term, 0, _heater_config.Ki_limit); // Integral term can't be negative in our case
  _derivative_term = constrain(_derivative_term, -_heater_config.Kd_limit, 0); // Derivative term can't be positive in our case 

  // Update temperature step temp value to nearest rounded down integer
  // if the container temp exceededs current temp step temp
  if (_derivative_term < 0 && container_temp - _current_temperature_step > 1 && _current_temperature_step != _heater_config.target_temp)
  {
    _current_temperature_step = floor(container_temp);
  }

  // Save last proportional term for future derivative term calculations
  _last_proportional_term = _proportional_term;

  // Save last PID calculation time for time delta calculations
  _last_pid_calculation_time = millis();
}

void Heater::calculateHeaterPwm()
{
  // Heater power is the sum of all the individual PID values multiplied by their coefficients
  _heater_pwm = _heater_config.Kp * _proportional_term;
  _heater_pwm += _heater_config.Ki * _integral_term;
  _heater_pwm += _heater_config.Kd * _derivative_term;

  // Constrain the final sum to PWM output range
  _heater_pwm = constrain(_heater_pwm, _heater_config.pwm_min, _heater_config.pwm_max);
}

// heater PWM value
void Heater::setHeaterPwm()
{
  analogWrite(_heater_config.heater_pin, (int)_heater_pwm);
}

void Heater::reset()
{
  _last_pid_calculation_time = millis();
  _time_at_temperature_step_start = millis();

  float _proportional_term = 0;
  float _last_proportional_term = 0;
  float _derivative_term = 0;
  float _integral_term = 0;
  
  float _current_temperature_step = 0;
  float _heater_pwm = 0;
  setHeaterPwm();
}