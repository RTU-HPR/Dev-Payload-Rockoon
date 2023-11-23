#include "data_filtering.h"
#include <Arduino.h>

// Create new time averaging filter object
template <typename T>
Time_Averaging_Filter<T>::Time_Averaging_Filter(unsigned int capacity, unsigned long averaging_time_ms)
{
    _capacity = capacity;
    _values = new T[capacity];
    _timestamps = new unsigned long[capacity];
    set_averaging_time(averaging_time_ms);
    reset_data();
}

// Destroy time averaging filter object
template <typename T>
Time_Averaging_Filter<T>::~Time_Averaging_Filter()
{
    delete[] _values;
    delete[] _timestamps;
}

// Set averaging time in milliseconds
template <typename T>
void Time_Averaging_Filter<T>::set_averaging_time(unsigned long time_ms)
{
    _averaging_time = time_ms;
}

// Reset current data stored in the filter
template <typename T>
void Time_Averaging_Filter<T>::reset_data()
{
    // Clear the data and timestamps arrays
    for (unsigned int i = 0; i < _capacity; i++)
    {
        _values[i] = T(); // Default constructor for type T (set to 0 for numeric types)
        _timestamps[i] = 0;
    }
}

// Add new value to the filter
template <typename T>
void Time_Averaging_Filter<T>::add_data(T data_point)
{
    // Find the oldest entry and replace it if the array is full
    unsigned int oldest_index = 0;
    unsigned long oldest_timestamp = _timestamps[0];

    for (unsigned int i = 1; i < _capacity; i++)
    {
        if (_timestamps[i] < oldest_timestamp)
        {
            oldest_index = i;
            oldest_timestamp = _timestamps[i];
        }
    }

    // Add the new data point and timestamp
    _values[oldest_index] = data_point;
    _timestamps[oldest_index] = millis();
}

// Return the current average filter value
template <typename T>
T Time_Averaging_Filter<T>::get_averaged_value()
{
    unsigned long current_time = millis();

    T sum = T(); // Initialize the sum to the default value for type T
    unsigned int count = 0;

    for (unsigned int i = 0; i < _capacity; i++)
    {
        if (current_time - _timestamps[i] <= _averaging_time)
        {
            sum += _values[i];
            count++;
        }
    }

    // Calculate the average
    if (count > 0)
    {
        return sum / static_cast<T>(count);
    }
    else
    {
        // Return a default value if there is no recent data
        return T();
    }
}
