// File Path: /lib/GuidedTuningEngine/src/GuidedTuningEngine.cpp
// MODIFIED FILE

#include "GuidedTuningEngine.h"
#include "../../src/DebugConfig.h"
#include <Arduino.h>
#include <arduinoFFT.h>

// Helper function to map a value from one range to another
double map_double(double x, double in_min, double in_max, double out_min, double out_max) {
    if (in_max - in_min == 0) return out_min; // Avoid division by zero
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

GuidedTuningEngine::GuidedTuningEngine() :
    _rawStdDev(0.0),
    _peakFrequency(0.0)
{
    for (int i = 0; i < GT_SAMPLE_COUNT; ++i) {
        _rawSamples[i] = 0.0;
    }
}

bool GuidedTuningEngine::proposeSettings(FilterManager* targetFilter, AdcManager* adcManager, uint8_t adcIndex, uint8_t adcInput) {
    if (!targetFilter || !adcManager) return false;

    LOG_AUTO_TUNE("Starting high-speed signal capture...");
    if (!captureSignal(adcManager, adcIndex, adcInput)) {
        LOG_AUTO_TUNE("Failed to capture signal.");
        return false;
    }
    LOG_AUTO_TUNE("Signal capture complete.");

    LOG_AUTO_TUNE("Analyzing signal...");
    analyzeSignal();
    LOG_AUTO_TUNE("Analysis complete. Raw StdDev: %.4f, Peak Freq: %.2f Hz", _rawStdDev, _peakFrequency);

    PI_Filter* hfFilter = targetFilter->getFilter(0);
    if (hfFilter) {
        LOG_AUTO_TUNE("Deriving HF filter parameters...");
        deriveHfParameters(hfFilter);
    }

    PI_Filter* lfFilter = targetFilter->getFilter(1);
    if (hfFilter && lfFilter) {
        LOG_AUTO_TUNE("Deriving LF filter parameters...");
        deriveLfParameters(hfFilter, lfFilter);
    }
    
    LOG_AUTO_TUNE("Guided Tuning complete. New parameters applied.");
    return true;
}

bool GuidedTuningEngine::captureSignal(AdcManager* adcManager, uint8_t adcIndex, uint8_t adcInput) {
    const int delay_between_samples_us = 1000;
    for (int i = 0; i < GT_SAMPLE_COUNT; ++i) {
        _rawSamples[i] = adcManager->getVoltage(adcIndex, adcInput);
        delayMicroseconds(delay_between_samples_us);
    }
    return (_rawSamples[GT_SAMPLE_COUNT / 2] != 0.0);
}

void GuidedTuningEngine::analyzeSignal() {
    double sum = 0.0;
    for (int i = 0; i < GT_SAMPLE_COUNT; ++i) {
        sum += _rawSamples[i];
    }
    double mean = sum / GT_SAMPLE_COUNT;

    for (int i = 0; i < GT_SAMPLE_COUNT; ++i) {
        _fftReal[i] = _rawSamples[i] - mean;
        _fftImag[i] = 0.0;
    }

    double sumSqDiff = 0.0;
    for (int i = 0; i < GT_SAMPLE_COUNT; ++i) {
        sumSqDiff += _fftReal[i] * _fftReal[i];
    }
    _rawStdDev = sqrt(sumSqDiff / GT_SAMPLE_COUNT);

    double sampling_frequency = 1000.0;
    arduinoFFT FFT = arduinoFFT(_fftReal, _fftImag, GT_SAMPLE_COUNT, sampling_frequency);

    FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(FFT_FORWARD);
    FFT.ComplexToMagnitude();
    _peakFrequency = FFT.MajorPeak();
}

/**
 * @brief --- DEFINITIVE FIX: Re-architected to a single-pass heuristic algorithm. ---
 * This new version calculates parameters directly from the signal analysis results,
 * completely eliminating the nested, memory-intensive simulation loops that were
 * causing the stack overflow crashes. This is a lean, fast, and RTOS-safe design.
 */
void GuidedTuningEngine::deriveHfParameters(PI_Filter* targetHfFilter) {
    if (!targetHfFilter) return;

    // Rule 1: Settle threshold is proportional to the overall noise amplitude.
    targetHfFilter->settleThreshold = _rawStdDev * 1.5;
    targetHfFilter->settleThreshold = constrain(targetHfFilter->settleThreshold, 0.01, 1.0);

    // Rule 2: Median window size is based on the dominant noise frequency.
    targetHfFilter->medianWindowSize = (_peakFrequency > 150) ? 7 : 5;

    // Rule 3: Track response is mapped directly to the noise frequency.
    // Faster noise (higher frequency) needs a faster filter response.
    targetHfFilter->trackResponse = map_double(_peakFrequency, 10.0, 500.0, 0.4, 0.8);
    targetHfFilter->trackResponse = constrain(targetHfFilter->trackResponse, 0.4, 0.8);

    // Rule 4: Lock smoothing is inversely proportional to the track response.
    // A fast-responding filter should have gentle smoothing to avoid over-correction.
    targetHfFilter->lockSmoothing = map_double(targetHfFilter->trackResponse, 0.4, 0.8, 0.2, 0.05);
    targetHfFilter->lockSmoothing = constrain(targetHfFilter->lockSmoothing, 0.05, 0.2);

    // Rule 5: Track assist is a small, constant value for the HF stage.
    targetHfFilter->trackAssist = 0.01;

    LOG_AUTO_TUNE("HF Results: Settle=%.3f, Resp=%.2f, Smooth=%.2f", targetHfFilter->settleThreshold, targetHfFilter->trackResponse, targetHfFilter->lockSmoothing);
}

/**
 * @brief --- DEFINITIVE FIX: Uses a single, stateful simulation of the HF filter. ---
 * This function now runs one clean simulation of the newly configured HF filter
 * to get its output characteristics, then calculates the LF parameters directly
 * from that cleaner signal, avoiding nested simulations and stack overflows.
 */
void GuidedTuningEngine::deriveLfParameters(PI_Filter* hfFilter, PI_Filter* targetLfFilter) {
    if (!hfFilter || !targetLfFilter) return;

    // Create a clean HF filter instance for a stateful simulation.
    PI_Filter hfSim(*hfFilter); // Use the safe copy constructor

    // Run the single simulation to get the characteristics of the HF stage's output.
    double hf_filtered_output[GT_SAMPLE_COUNT];
    for(int i=0; i < GT_SAMPLE_COUNT; ++i){
        hf_filtered_output[i] = hfSim.process(_rawSamples[i]);
    }
    
    // Analyze the now-cleaner signal from the HF stage.
    double hf_sum = 0.0;
    for(int i=0; i<GT_SAMPLE_COUNT; ++i) hf_sum += hf_filtered_output[i];
    double hf_mean = hf_sum / GT_SAMPLE_COUNT;
    double hf_sum_sq_diff = 0.0;
    for(int i=0; i<GT_SAMPLE_COUNT; ++i) hf_sum_sq_diff += (hf_filtered_output[i] - hf_mean) * (hf_filtered_output[i] - hf_mean);
    double hf_output_std_dev = sqrt(hf_sum_sq_diff / GT_SAMPLE_COUNT);

    // Now, calculate the LF parameters to aggressively smooth this cleaner signal.
    targetLfFilter->settleThreshold = hf_output_std_dev * 0.5;
    targetLfFilter->settleThreshold = constrain(targetLfFilter->settleThreshold, 0.001, 0.5);
    targetLfFilter->medianWindowSize = 15;
    targetLfFilter->trackResponse = 0.05;
    targetLfFilter->lockSmoothing = 0.005;
    targetLfFilter->trackAssist = 0.0001;

    LOG_AUTO_TUNE("LF Results: Settle=%.3f, Resp=%.2f, Smooth=%.3f", targetLfFilter->settleThreshold, targetLfFilter->trackResponse, targetLfFilter->lockSmoothing);
}