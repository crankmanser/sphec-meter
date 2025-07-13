// src/managers/diagnostics/NoiseAnalysisManager.cpp
// NEW FILE
#include "NoiseAnalysisManager.h"
#include "DebugMacros.h"
#include <vector>

// These constants are from SensorProcessor.cpp and are needed for voltage conversion
const float VOLTAGE_DIVIDER_COMPENSATION_N = 2.0f;
const float ADC_FSR_PROBES_N = 4.096f;

NoiseAnalysisManager::NoiseAnalysisManager(ADS1118_Driver* adc1, ADS1118_Driver* adc2) :
    _adc1(adc1),
    _adc2(adc2)
{}

StatisticalResult NoiseAnalysisManager::runAnalysis(SensorToAnalyze sensor) {
    LOG_MANAGER("--- Starting Noise Analysis for %s ---\n", (sensor == SensorToAnalyze::PH_PROBE) ? "PH" : "EC");

    if (!_adc1 || !_adc2) {
        LOG_MAIN("[NAM_ERROR] ADC driver is null. Aborting analysis.\n");
        return StatisticalResult();
    }

    std::vector<float> samples;
    samples.reserve(SAMPLE_COUNT);

    // --- Data Capture Loop ---
    LOG_MANAGER("Capturing %d samples at %d Hz...\n", SAMPLE_COUNT, SAMPLING_RATE_HZ);
    for (int i = 0; i < SAMPLE_COUNT; ++i) {
        int16_t raw_value = 0;
        if (sensor == SensorToAnalyze::PH_PROBE) {
            raw_value = _adc1->readDifferential_0_1();
        } else {
            raw_value = _adc2->readDifferential_0_1();
        }
        
        float voltage = adcValueToVoltage(raw_value);
        samples.push_back(voltage);
        
        delayMicroseconds(SAMPLING_INTERVAL_US);
    }
    LOG_MANAGER("Sample capture complete.\n");

    // --- Analysis ---
    StatisticalResult results = StatisticalAnalyzer::analyze(samples);

    // --- Log Results for Verification ---
    if (results.is_valid) {
        LOG_MANAGER("--- Noise Analysis Results ---\n");
        LOG_MANAGER("  Sample Count: %u\n", results.sample_count);
        LOG_MANAGER("  Mean Voltage: %.4f V\n", results.mean);
        LOG_MANAGER("  Min Voltage:  %.4f V\n", results.min_val);
        LOG_MANAGER("  Max Voltage:  %.4f V\n", results.max_val);
        LOG_MANAGER("  Peak-to-Peak: %.4f V\n", results.peak_to_peak);
        LOG_MANAGER("  Std. Dev.:    %.6f V\n", results.std_dev);
        LOG_MANAGER("-----------------------------\n");
    } else {
        LOG_MAIN("[NAM_ERROR] Analysis failed, not enough data.\n");
    }

    return results;
}

float NoiseAnalysisManager::adcValueToVoltage(int16_t raw_value) {
    // This conversion logic is identical to the one in SensorProcessor
    float voltage = (static_cast<float>(raw_value) / 32767.0f) * ADC_FSR_PROBES_N;
    return voltage * VOLTAGE_DIVIDER_COMPENSATION_N;
}