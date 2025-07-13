// src/helpers/diagnostics/StatisticalAnalyzer.cpp
// NEW FILE
#include "StatisticalAnalyzer.h"
#include <numeric>
#include <cmath>
#include <algorithm>

StatisticalResult StatisticalAnalyzer::analyze(const std::vector<float>& samples) {
    StatisticalResult result;
    result.sample_count = samples.size();

    if (result.sample_count < 2) {
        return result; // Not enough data to be statistically valid
    }

    // --- Calculate Mean, Min, and Max in a single pass ---
    float sum = 0.0f;
    result.min_val = samples[0];
    result.max_val = samples[0];
    for (const auto& sample : samples) {
        sum += sample;
        if (sample < result.min_val) result.min_val = sample;
        if (sample > result.max_val) result.max_val = sample;
    }
    result.mean = sum / result.sample_count;
    result.peak_to_peak = result.max_val - result.min_val;

    // --- Calculate Standard Deviation ---
    float sum_sq_diff = 0.0f;
    for (const auto& sample : samples) {
        sum_sq_diff += (sample - result.mean) * (sample - result.mean);
    }
    float variance = sum_sq_diff / (result.sample_count - 1); // Sample variance
    result.std_dev = std::sqrt(variance);

    result.is_valid = true;
    return result;
}