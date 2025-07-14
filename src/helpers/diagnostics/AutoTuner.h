// src/helpers/diagnostics/AutoTuner.h
// NEW FILE
#pragma once

#include "helpers/filters/PIFilter.h"
#include "helpers/diagnostics/StatisticalAnalyzer.h"
#include "helpers/diagnostics/FftAnalyzer.h"

/**
 * @class AutoTuner
 * @brief A stateless helper cabinet for deriving optimal filter parameters.
 *
 * This class will take the results from the Statistical and FFT analyzers
 * and use them to calculate the ideal tuning parameters for the various
 * software filters (e.g., Notch filter, PIFilter) to be applied to the
 * signal chain.
 */
class AutoTuner {
public:
    /**
     * @brief Derives the optimal PIFilter tuning parameters from statistical results.
     * @param stats The StatisticalResult from the initial analysis.
     * @return A PIFilter::Tuni_t struct with the calculated parameters.
     */
    static PIFilter::Tuni_t tunePiFilter(const StatisticalResult& stats);

    // Future methods can be added for other filter types, e.g.,
    // static NotchFilterParams tuneNotchFilter(const FftResult& fft_result);
};