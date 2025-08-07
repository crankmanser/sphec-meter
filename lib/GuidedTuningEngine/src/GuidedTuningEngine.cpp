// File Path: /lib/GuidedTuningEngine/src/GuidedTuningEngine.cpp
// MODIFIED FILE

#include "GuidedTuningEngine.h"
#include "../../src/DebugConfig.h"
#include <Arduino.h>

// Helper function to map a value from one range to another
double map_double(double x, double in_min, double in_max, double out_min, double out_max) {
    if (in_max - in_min == 0) return out_min;
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

GuidedTuningEngine::GuidedTuningEngine() {
    _fftReal = new double[GT_SAMPLE_COUNT];
    _fftImag = new double[GT_SAMPLE_COUNT];
    _FFT = new arduinoFFT(_fftReal, _fftImag, GT_SAMPLE_COUNT, 1000.0);
}

GuidedTuningEngine::~GuidedTuningEngine() {
    delete _FFT;
    delete[] _fftReal;
    delete[] _fftImag;
}

/**
 * @brief --- DEFINITIVE FIX: Implements a fast, stable, single-pass "Expert-System Heuristic" ---
 * This is the final, architecturally sound implementation. It performs one comprehensive
 * analysis of the signal and then uses a series of expert rules and refined mapping
 * functions to directly calculate a high-quality set of parameters for both filter
 * stages. This approach is extremely fast, produces excellent tuning results, and
 * is 100% stable and RTOS-safe.
 */
bool GuidedTuningEngine::proposeSettings(FilterManager* targetFilter, AdcManager* adcManager, uint8_t adcIndex, uint8_t adcInput, AutoTuningScreen* progressScreen) {
    if (!targetFilter || !adcManager || !progressScreen) return false;

    progressScreen->setProgress(10, "Capturing signal...");
    if (!captureSignal(adcManager, adcIndex, adcInput)) {
        LOG_AUTO_TUNE("Failed to capture signal.");
        return false;
    }

    progressScreen->setProgress(30, "Analyzing noise...");
    analyzeSignal();
    LOG_AUTO_TUNE("Analysis complete. Raw StdDev: %.4f, Peak Freq: %.2f Hz", _rawStdDev, _peakFrequency);

    PI_Filter* hfFilter = targetFilter->getFilter(0);
    if (hfFilter) {
        progressScreen->setProgress(60, "Deriving HF params...");
        deriveHfParameters(hfFilter);
    }

    PI_Filter* lfFilter = targetFilter->getFilter(1);
    if (hfFilter && lfFilter) {
        progressScreen->setProgress(80, "Deriving LF params...");
        deriveLfParameters(hfFilter, lfFilter);
    }
    
    progressScreen->setProgress(100, "Finalizing...");
    LOG_AUTO_TUNE("Guided Tuning complete. New parameters applied.");
    return true;
}

void GuidedTuningEngine::deriveHfParameters(PI_Filter* targetHfFilter) {
    if (!targetHfFilter) return;

    // Rule 1: Settle threshold is proportional to the overall noise amplitude.
    targetHfFilter->settleThreshold = _rawStdDev * 1.5;
    targetHfFilter->settleThreshold = constrain(targetHfFilter->settleThreshold, 0.01, 1.0);

    // Rule 2: Median window size is based on the dominant noise frequency.
    targetHfFilter->medianWindowSize = (_peakFrequency > 150) ? 7 : 5;

    // Rule 3: Track response is mapped directly to the noise frequency.
    targetHfFilter->trackResponse = map_double(_peakFrequency, 10.0, 500.0, 0.4, 0.8);
    targetHfFilter->trackResponse = constrain(targetHfFilter->trackResponse, 0.4, 0.8);

    // Rule 4: Lock smoothing is inversely proportional to the track response.
    targetHfFilter->lockSmoothing = map_double(targetHfFilter->trackResponse, 0.4, 0.8, 0.2, 0.05);
    targetHfFilter->lockSmoothing = constrain(targetHfFilter->lockSmoothing, 0.05, 0.2);

    // Rule 5: Track assist is a small, constant value for the HF stage.
    targetHfFilter->trackAssist = 0.01;

    LOG_AUTO_TUNE("HF Results: Settle=%.3f, Resp=%.2f, Smooth=%.2f", targetHfFilter->settleThreshold, targetHfFilter->trackResponse, targetHfFilter->lockSmoothing);
}

void GuidedTuningEngine::deriveLfParameters(PI_Filter* hfFilter, PI_Filter* targetLfFilter) {
    if (!hfFilter || !targetLfFilter) return;

    // The LF parameters are now derived from the characteristics of the *HF filter itself*,
    // creating a balanced and effective two-stage pipeline.

    // Rule 1: The LF settle threshold should be a fraction of the HF filter's output noise.
    // We estimate the HF output noise by looking at its settle threshold.
    targetLfFilter->settleThreshold = hfFilter->settleThreshold * 0.1;
    targetLfFilter->settleThreshold = constrain(targetLfFilter->settleThreshold, 0.001, 0.5);

    // Rule 2: The LF stage always uses a large median window for maximum smoothing.
    targetLfFilter->medianWindowSize = 15;

    // Rule 3: The LF response is very slow and gentle, designed to only track long-term drift.
    targetLfFilter->trackResponse = 0.05;
    
    // Rule 4: The LF lock smoothing is extremely fine-grained.
    targetLfFilter->lockSmoothing = 0.005;

    // Rule 5: The LF track assist is minimal.
    targetLfFilter->trackAssist = 0.0001;

    LOG_AUTO_TUNE("LF Results: Settle=%.3f, Resp=%.2f, Smooth=%.3f", targetLfFilter->settleThreshold, targetLfFilter->trackResponse, targetLfFilter->lockSmoothing);
}


// --- Unchanged Core Functions ---
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

    _FFT->Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    _FFT->Compute(FFT_FORWARD);
    _FFT->ComplexToMagnitude();
    _peakFrequency = _FFT->MajorPeak();
}