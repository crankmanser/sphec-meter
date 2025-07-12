#pragma once

#include <Arduino.h>
#include <vector>
#include <algorithm>

class PIFilter {
public:
    // This struct holds all the tunable parameters for the filter,
    // using the exact terminology from the legacy system and user manual.
    struct Tuni_t {
        float settle_threshold;
        float lock_smoothing;
        float track_response;
        float track_assist;
    };

private:
    // The internal operational states of the filter.
    enum class FilterState {
        LOCKED,
        TRACKING
    };

public:
    // The constructor takes the median filter window size and the tuning parameters.
    PIFilter(uint8_t window_size, const Tuni_t& tuning);

    // Adds a new raw measurement and returns the new filtered value.
    float update(float new_value);

    // Reconfigures the filter's parameters on-the-fly.
    void reconfigure(const Tuni_t& tuning);

    // Accessor methods to get the filter's output and state.
    float getValue() const;
    float getStabilityFactor() const;
    FilterState getState() const;

private:
    // --- Configuration ---
    uint8_t _window_size;
    Tuni_t _tuning;
    float _lock_threshold;
    float _track_threshold;

    // --- Internal State ---
    float _p_term; // The Proportional (main filtered value) term
    float _i_term; // The Integral (offset correction) term
    float _stability_factor;
    float _smoothed_trend;
    bool _primed;
    FilterState _state;
    std::vector<float> _buffer; // The internal buffer for the median filter
};