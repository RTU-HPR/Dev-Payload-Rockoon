#include "temperature_manager.h"

// main function to call every
void Temperature_Manager::update_heater_power(float inner_temp)
{
    _inner_temp = inner_temp;
    calculate_heater_power();
    set_heater_power(_heater_power);
}

// Safety check incase the pid loop did an opsie
void Temperature_Manager::check_heater_power()
{
    if (_inner_temp > (_safe_temp + 0.05))
    {
        _heater_power = 0;
    }
}

void Temperature_Manager::calculate_heater_power()
{
    // Time since last pid update
    float dt = (millis() - _last_pid_calculation_time);

    // If the temperature is 0.1 degree below or above the safe temp, reset the timer
    if ((_safe_temp - 0.1) > _inner_temp || _inner_temp > (_safe_temp + 0.1))
    {
        _time_at_safe_temp_start = millis();
    }

    // Calculate proportional term based on the state
    if (_safe_temp == _desired_temp)
    {
        _proportional_term = _desired_temp - _inner_temp;
    }
    else
    {
        // If the temperature has stayed at the safe temp for 20 seconds, increase the safe temperature
        if ((millis() - _time_at_safe_temp_start) > 20000)
        {
            _safe_temp += 1;
            _time_at_safe_temp_start = millis();
        }
        _proportional_term = _safe_temp - _inner_temp;
    }

    // Integral term
    // Don't allow integral term to be negative
    if (_integral_term < 0)
    {
        _integral_term = 0;
    }
    else
    {
        _integral_term += _proportional_term * dt;
    }

    // Derivative term
    _derivative_term = (_proportional_term - _last_proportional_term) / dt;

    // Heater power is the sum of all the individual values multiplied by their coefficients
    // The values need to be constrained to not become too extreme
    _heater_power = constrain(_Kp * _proportional_term, -_proportional_limit, _proportional_limit);
    _heater_power += constrain(_Ki * _integral_term, 0, _integral_limit);
    _heater_power += constrain(_Kd * _derivative_term, -_derivative_limit, 0);

    // Constrain the final sum to PWM output range
    // Limit the heater power to half, for the first minutes
    if (millis() - _heater_turn_on_time > _startup_pwm_limit_time)
    {
        _heater_power = constrain(_heater_power, _pwm_min, _pwm_max / 2);
    }
    else
    {
        _heater_power = constrain(_heater_power, _pwm_min, _pwm_max);
    }

    // Save last proportional term for future derivative term calculations
    _last_proportional_term = _proportional_term;

    // Save last PID calculation time for time delta calculations
    _last_pid_calculation_time = millis();
}

// Return current heater power
double Temperature_Manager::get_heater_power()
{
    return _heater_power;
}

// heater PWM value
void Temperature_Manager::set_heater_power(float heater_power_pwm)
{
    pinMode(_heater_pin, OUTPUT_12MA);
    analogWrite(_heater_pin, heater_power_pwm);
}

// reset the last calculation times and set the mosfet to 0. Call this if ending the update loop or restarting it if there has been a long pause
void Temperature_Manager::reset()
{
    _last_pid_calculation_time = millis();
    _heater_turn_on_time = millis();
    _heater_power = 0;
    set_heater_power(_heater_power);
}

// reset the last calculation times and set the mosfet to 0. Call this if ending the update loop or restarting it if there has been a long pause
void Temperature_Manager::reset_from_state(float integral_term, float safe_temp)
{
    _integral_term = integral_term;
    _safe_temp = safe_temp;
    _heater_power = 0;
    set_heater_power(_heater_power);
}

// Get current PID values
void Temperature_Manager::get_pid(float &p, float &i, float &d)
{
    p = _proportional_term * _Kp;
    i = _integral_term * _Ki;
    d = _derivative_term * _Kd;
}
// initialise the main values
Temperature_Manager::Temperature_Manager(int heater_pin, float desired_temp)
{
    _heater_pin = heater_pin;
    _desired_temp = desired_temp;
    _safe_temp = _desired_temp - 5;
}

Temperature_Manager::~Temperature_Manager()
{
    reset();
}