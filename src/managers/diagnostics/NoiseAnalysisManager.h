// src/managers/diagnostics/NoiseAnalysisManager.h
// NEW FILE
#pragma once

#include "hal/ADS1118_Driver.h"
#include "helpers/diagnostics/StatisticalAnalyzer.h" // Include the helper

// Enum to specify which sensor's signal to analyze.
enum class SensorToAnalyze {
    PH_PROBE,
    EC_PROBE
};

class NoiseAnalysisManager {
public:
    /**
     * @brief Constructs the NoiseAnalysisManager.
     * @param adc1 Pointer to the ADS1118 driver for the pH probe.
     * @param adc2 Pointer to the ADS1118 driver for the EC probe.
     */
    NoiseAnalysisManager(ADS1118_Driver* adc1, ADS1118_Driver* adc2);

    /**
     * @brief Runs a complete noise analysis on the specified sensor.
     * This is a blocking function that performs sampling and analysis.
     * @param sensor The sensor to analyze (PH_PROBE or EC_PROBE).
     * @return A StatisticalResult struct with the analysis results.
     */
    StatisticalResult runAnalysis(SensorToAnalyze sensor);

private:
    ADS1118_Driver* _adc1;
    ADS1118_Driver* _adc2;

    // Constants for the analysis window
    static constexpr int SAMPLE_COUNT = 1024;
    static constexpr int SAMPLING_RATE_HZ = 250;
    static constexpr int SAMPLING_INTERVAL_US = 1000000 / SAMPLING_RATE_HZ;

    /**
     * @brief Converts a raw 16-bit ADC value to a voltage.
     * @param raw_value The raw value from the ADC.
     * @return The calculated voltage.
     */
    float adcValueToVoltage(int16_t raw_value);
};