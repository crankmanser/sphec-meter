#pragma once

#include "helpers/filters/PIFilter.h" // Needed for PIFilter::Tuni_t

namespace FilterTuningDefaults {

    // --- Default PI Filter Tuning Parameters for pH Sensor ---
    // These values are sourced conceptually from the "Advanced Filter User Manual & Tuning Guide".
    // They serve as the starting point for filter behavior if no custom tuning is saved on SD.
    const PIFilter::Tuni_t DEFAULT_PH_FILTER_TUNING = {
        0.005f, // settle_threshold: (e.g., 5mV) - how close new reading must be to filtered value to consider "settled"
        0.05f,  // lock_smoothing: how aggressively to smooth when signal is "locked" (stable) - higher = more smoothing
        0.9f,   // track_response: how quickly to respond when signal is "tracking" (changing) - higher = faster response
        0.001f  // track_assist: small integral gain to help nudge filter towards true value when tracking
    };

    // --- Default PI Filter Tuning Parameters for EC Sensor ---
    const PIFilter::Tuni_t DEFAULT_EC_FILTER_TUNING = {
        0.01f,  // settle_threshold: (e.g., 10mV)
        0.03f,  // lock_smoothing
        0.85f,  // track_response
        0.0005f // track_assist
    };

} // namespace FilterTuningDefaults