// src/helpers/diagnostics/StatisticalAnalyzer.h
// NEW FILE
#pragma once

#include <vector>
#include <cstdint>

// A simple struct to hold the results of a statistical analysis.
struct StatisticalResult {
    bool is_valid = false;
    float mean = 0.0f;
    float std_dev = 0.0f;
    float min_val = 0.0f;
    float max_val = 0.0f;
    float peak_to_peak = 0.0f;
    uint32_t sample_count = 0;
};

class StatisticalAnalyzer {
public:
    /**
     * @brief Calculates statistical metrics for a given set of samples.
     * * @param samples A vector of floating-point sample values.
     * @return A StatisticalResult struct containing the calculated metrics.
     */
    static StatisticalResult analyze(const std::vector<float>& samples);
};