#pragma once

#include <Arduino.h>
#include <vector>
#include <algorithm>

class MedianFilter {
public:
    // Constructor: Defines the size of the sample window.
    MedianFilter(size_t window_size);

    // Adds a new raw measurement to the filter's sample window.
    void add(float value);

    // Calculates and returns the median value from the current window.
    float getValue() const;

private:
    std::vector<float> _window;
    size_t _current_position;
    bool _is_filled;
};