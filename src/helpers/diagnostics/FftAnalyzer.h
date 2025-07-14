// src/helpers/diagnostics/FftAnalyzer.h
// NEW FILE
#pragma once

#include <vector>
#include <complex>

// This struct holds the results of an FFT analysis.
struct FftResult {
    bool is_valid = false;
    std::vector<double> frequencies; // The frequency for each bin
    std::vector<double> magnitudes;  // The magnitude of each frequency component
    float dominant_frequency = 0.0f; // The frequency with the highest magnitude
};

/**
 * @class FftAnalyzer
 * @brief A stateless helper cabinet for performing Fast Fourier Transform analysis.
 *
 * This class will take a series of time-domain samples and convert them
 * into the frequency domain, allowing for the identification of periodic noise
 * such as 50/60Hz mains hum.
 */
class FftAnalyzer {
public:
    /**
     * @brief Performs an FFT on the given sample data.
     * @param samples A vector of time-domain sample values.
     * @param sampling_rate_hz The rate at which the samples were taken.
     * @return An FftResult struct containing the analysis.
     */
    static FftResult analyze(const std::vector<float>& samples, int sampling_rate_hz);
};