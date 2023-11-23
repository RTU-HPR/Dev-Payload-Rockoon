#include "ClosedCube_STS35.h"
#include "ClosedCube_I2C.h"

class Temperature_Manager
{
private:
    // Heating
    int _heater_pin;
    float _desired_temp; // in C
    
    const int _pwm_min = 0;
    const int _pwm_max = 32;

    // PID coefficients
    const float _Kp = 7.5;     // Proportional (Best between 5 and 15)
    const float _Ki = 0.00001; // Integral (Best between 0.00008 and 0.00013)
    const float _Kd = 10000;   // Derivative (Best between 5000 and 20000)

    // PID value limits
    const int _proportional_limit = _pwm_max;
    const int _integral_limit = _pwm_max;
    const int _derivative_limit = _pwm_max / 2;

    // Timing
    unsigned long int _heater_turn_on_time = 0;
    unsigned long int _startup_pwm_limit_time = 1000 * 2 * 60; // 2 minutes
    unsigned long int _last_pid_calculation_time = 0;
    unsigned long int _time_at_safe_temp_start = 0;

    float _inner_temp = 0;
    float _heater_power = 0;
    void set_heater_power(float heater_power_pwm);
    void calculate_heater_power();
    void check_heater_power();

public:
    bool _heater_enabled = false;
    float _safe_temp;

    // PID values
    float _proportional_term = 0;
    float _last_proportional_term = 0;
    float _derivative_term = 0;
    float _integral_term = 0;

    Temperature_Manager(int heater_pin, float desired_temp);
    ~Temperature_Manager();
    // TODO properly reset all the necessary values
    void reset();
    void reset_from_state(float integral_term, float safe_temp);
    void update_heater_power(float inner_temp);
    double get_heater_power();
    void get_pid(float &p, float &i, float &d);
    float get_target_temp() { return _safe_temp; }
};