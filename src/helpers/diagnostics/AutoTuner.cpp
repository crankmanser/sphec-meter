// src/helpers/diagnostics/AutoTuner.cpp
// NEW FILE
#include "AutoTuner.h"
#include <algorithm>

PIFilter::Tuni_t AutoTuner::tunePiFilter(const StatisticalResult& stats) {
    PIFilter::Tuni_t tuning;

    if (!stats.is_valid) {
        return tuning; // Return default (empty) tuning
    }

    // --- DERIVE 'settle_threshold' ---
    // A reasonable starting point is 3x the standard deviation of the noise.
    // This represents a threshold that a truly settled signal should stay within.
    tuning.settle_threshold = std::max(0.001f, 3.0f * stats.std_dev);

    // --- DERIVE 'lock_smoothing' (Alpha for Locked State) ---
    // This value determines how much smoothing is applied when the signal is stable.
    // A higher value means more aggressive smoothing. We can base this on the noise level.
    // Map std_dev from a range (e.g., 0 to 0.01) to a smoothing range (e.g., 0.1 to 0.01)
    float lock_smoothing_mapped = 0.1f - (stats.std_dev / 0.01f) * 0.09f;
    tuning.lock_smoothing = std::max(0.01f, std::min(lock_smoothing_mapped, 0.1f));

    // --- DERIVE 'track_response' (Alpha for Tracking State) ---
    // This should be high to allow the filter to respond quickly to real changes.
    // A value between 0.8 and 0.95 is typical. We'll use a fixed high value.
    tuning.track_response = 0.9f;

    // --- DERIVE 'track_assist' (Integral Gain) ---
    // This helps eliminate steady-state error. A small value is best.
    // We can make it proportional to the settle threshold.
    tuning.track_assist = tuning.settle_threshold * 0.1f;

    return tuning;
}