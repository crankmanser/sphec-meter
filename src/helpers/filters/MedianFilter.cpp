#include "MedianFilter.h"

MedianFilter::MedianFilter(size_t window_size) :
    _window(window_size),
    _current_position(0),
    _is_filled(false)
{}

void MedianFilter::add(float value) {
    if (isnan(value)) {
        return; // Do not add invalid numbers to the window
    }

    _window[_current_position] = value;
    _current_position++;
    if (_current_position >= _window.size()) {
        _current_position = 0;
        _is_filled = true;
    }
}

float MedianFilter::getValue() const {
    if (_window.empty()) {
        return 0.0f;
    }

    std::vector<float> sorted_window;

    // Determine the actual number of valid elements to sort
    size_t elements_to_sort = _is_filled ? _window.size() : _current_position;

    if (elements_to_sort == 0) {
        return 0.0f;
    }
    
    // Copy the valid elements into a temporary vector for sorting
    sorted_window.assign(_window.begin(), _window.begin() + elements_to_sort);
    
    // Sort the temporary vector to find the median
    std::sort(sorted_window.begin(), sorted_window.end());

    // Return the middle value
    return sorted_window[sorted_window.size() / 2];
}