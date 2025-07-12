#include "PIFilter.h"

// The constructor initializes the filter's state and calculates the hysteresis thresholds.
PIFilter::PIFilter(uint8_t window_size, const Tuni_t& tuning)
    : _window_size(window_size),
      _tuning(tuning),
      _p_term(0.0f),
      _i_term(0.0f),
      _stability_factor(0.0f),
      _smoothed_trend(0.0f),
      _primed(false),
      _state(FilterState::TRACKING) // Start in TRACKING to adapt to the initial signal
{
    // Ensure the median filter window is an odd number for a true median.
    if (_window_size % 2 == 0) {
        _window_size++;
    }
    _buffer.reserve(_window_size);
    // Initialize buffer with zeros to ensure median can be calculated on partial fill if needed
    // However, the current logic of returning new_value or _p_term + _i_term handles this
    // _buffer.assign(_window_size, 0.0f); // Not strictly needed with the proposed change below

    // Calculate the thresholds for state switching, creating the hysteresis gap.
    _lock_threshold = _tuning.settle_threshold * 0.1f;
    _track_threshold = _tuning.settle_threshold * 0.3f;
}

// This is the main update function, a direct migration of the legacy filter's logic.
float PIFilter::update(float new_value) {
    // --- Stage 1: Median Filter ---
    _buffer.push_back(new_value);
    if (_buffer.size() > _window_size) {
        _buffer.erase(_buffer.begin());
    }

    // --- Stage 2: Stateful PI Filter ---
    if (!_primed) {
        // Continue filling the buffer. If buffer is not yet full, return the raw new_value.
        // If buffer becomes full, prime the filter with the initial median.
        if (_buffer.size() < _window_size) {
            // Return the raw value until the buffer is full and the filter is primed.
            return new_value; 
        } else {
            // Buffer is now full, prime the filter.
            std::vector<float> sorted_buffer = _buffer;
            std::sort(sorted_buffer.begin(), sorted_buffer.end());
            _p_term = sorted_buffer[_window_size / 2]; // Initialize _p_term with the first median
            _primed = true; // Mark as primed
            _i_term = 0; // Reset integral term when priming
            return _p_term + _i_term; // Return the initial filtered value
        }
    } else {
        // Filter is already primed, proceed with normal PI logic
        std::vector<float> sorted_buffer = _buffer;
        std::sort(sorted_buffer.begin(), sorted_buffer.end());
        float median_value = sorted_buffer[_window_size / 2];

        // Calculate the raw trend and then smooth it to get a stable volatility metric.
        float raw_trend = abs(median_value - _p_term);
        _smoothed_trend = (0.05f * raw_trend) + (0.95f * _smoothed_trend);

        // State machine with hysteresis.
        if (_state == FilterState::TRACKING && _smoothed_trend < _lock_threshold) {
            _state = FilterState::LOCKED;
            _i_term = 0; // Reset integral term when locked.
        } else if (_state == FilterState::LOCKED && _smoothed_trend > _track_threshold) {
            _state = FilterState::TRACKING;
        }

        // Calculate stability factor for external use (e.g., UI).
        float normalized_trend = constrain(_smoothed_trend / _tuning.settle_threshold, 0.0f, 1.0f);
        _stability_factor = 1.0f - normalized_trend;

        float current_alpha;
        if (_state == FilterState::LOCKED) {
            current_alpha = _tuning.lock_smoothing;
        } else { // TRACKING
            current_alpha = _tuning.track_response;
            float error = median_value - _p_term;
            _i_term += error * _tuning.track_assist;
            // Anti-windup for the integral term.
            _i_term = constrain(_i_term, -_tuning.settle_threshold, _tuning.settle_threshold);
        }

        // Apply the core Proportional (P) filter.
        _p_term = (current_alpha * median_value) + (1.0f - current_alpha) * _p_term;
        return _p_term + _i_term; // Return the final value (P + I).
    }
}

// Allows for on-the-fly adjustment of filter parameters.
void PIFilter::reconfigure(const Tuni_t& tuning) {
    _tuning = tuning;
    _lock_threshold = _tuning.settle_threshold * 0.1f;
    _track_threshold = _tuning.settle_threshold * 0.3f;
    _primed = false; // Force re-priming with new settings
}

float PIFilter::getValue() const {
    return _p_term + _i_term;
}

float PIFilter::getStabilityFactor() const {
    return _stability_factor;
}

PIFilter::FilterState PIFilter::getState() const {
    return _state;
}