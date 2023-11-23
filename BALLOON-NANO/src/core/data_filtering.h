#pragma once

// Averages values over some time span, Time is managed inside the library.
template <typename T>
class Time_Averaging_Filter
{
private:
    T *_values;                    // An array to store data points
    unsigned long *_timestamps;    // An array to store timestamps
    unsigned long _averaging_time; // Time span for averaging in milliseconds
    unsigned int _capacity;        // Maximum number of data points

public:
    Time_Averaging_Filter(unsigned int capacity, unsigned long averaging_time_ms);
    ~Time_Averaging_Filter();

    // Function declarations
    void set_averaging_time(unsigned long time_ms);
    void reset_data();
    void add_data(T data_point);
    T get_averaged_value();
};

template class Time_Averaging_Filter<int>;
template class Time_Averaging_Filter<float>;