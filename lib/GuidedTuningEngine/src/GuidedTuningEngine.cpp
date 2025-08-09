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

bool GuidedTuningEngine::captureSignal(PBiosContext& context, AdcManager& adcManager, AutoTuningScreen& progressScreen) {
    progressScreen.setProgress(10, "Capturing Signal...");
    context.captured_samples.clear();
    context.captured_samples.reserve(GT_SAMPLE_COUNT);
    
    const int delay_between_samples_us = 1000;
    for (int i = 0; i < GT_SAMPLE_COUNT; ++i) {
        double voltage = adcManager.getVoltage(context.selectedAdcIndex, context.selectedAdcInput);
        context.captured_samples.push_back(voltage);
        delayMicroseconds(delay_between_samples_us);
    }
    
    bool success = (context.captured_samples.size() == GT_SAMPLE_COUNT);
    LOG_AUTO_TUNE("Signal capture complete. Success: %d", success);
    return success;
}

bool GuidedTuningEngine::characterizeSignal(PBiosContext& context, AutoTuningScreen& progressScreen) {
    progressScreen.setProgress(30, "Analyzing Noise...");
    if (context.captured_samples.size() != GT_SAMPLE_COUNT) return false;

    double sum = 0.0;
    for (const auto& sample : context.captured_samples) sum += sample;
    double mean = sum / GT_SAMPLE_COUNT;

    double sumSqDiff = 0.0;
    for (int i = 0; i < GT_SAMPLE_COUNT; ++i) {
        _fftReal[i] = context.captured_samples[i] - mean;
        _fftImag[i] = 0.0;
        sumSqDiff += _fftReal[i] * _fftReal[i];
    }
    context.raw_std_dev = sqrt(sumSqDiff / GT_SAMPLE_COUNT);

    _FFT->Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    _FFT->Compute(FFT_FORWARD);
    _FFT->ComplexToMagnitude();
    context.peak_frequency = _FFT->MajorPeak();
    
    LOG_AUTO_TUNE("Analysis complete. Raw StdDev: %.4f, Peak Freq: %.2f Hz", context.raw_std_dev, context.peak_frequency);
    return true;
}

bool GuidedTuningEngine::optimizeHfStage(PBiosContext& context, AutoTuningScreen& progressScreen) {
    progressScreen.setProgress(60, "Optimizing HF Stage...");
    if (!context.selectedFilter) return false;

    PI_Filter* hfFilter = context.selectedFilter->getFilter(0);
    if (!hfFilter) return false;
    
    // --- DEFINITIVE FIX: The erroneous reset is removed. ---
    // The engine now correctly modifies the *currently active* parameters,
    // making the process stateful and iterative.
    // *hfFilter = context.hf_params_snapshot; // This was the bug.
    
    hfFilter->settleThreshold = context.raw_std_dev * 1.5;
    hfFilter->settleThreshold = constrain(hfFilter->settleThreshold, 0.01, 1.0);
    hfFilter->medianWindowSize = (context.peak_frequency > 150) ? 7 : 5;
    hfFilter->trackResponse = map_double(context.peak_frequency, 10.0, 500.0, 0.4, 0.95);
    hfFilter->trackResponse = constrain(hfFilter->trackResponse, 0.4, 0.95);
    hfFilter->lockSmoothing = map_double(hfFilter->trackResponse, 0.4, 0.95, 0.2, 0.01);
    hfFilter->lockSmoothing = constrain(hfFilter->lockSmoothing, 0.01, 0.2);
    hfFilter->trackAssist = 0.01;

    LOG_AUTO_TUNE("HF Stage Optimized. Settle=%.3f, Resp=%.2f", hfFilter->settleThreshold, hfFilter->trackResponse);
    return true;
}

bool GuidedTuningEngine::optimizeLfStage(PBiosContext& context, AutoTuningScreen& progressScreen) {
    progressScreen.setProgress(80, "Optimizing LF Stage...");
    if (!context.selectedFilter) return false;
    
    PI_Filter* hfFilter = context.selectedFilter->getFilter(0);
    PI_Filter* lfFilter = context.selectedFilter->getFilter(1);
    if (!hfFilter || !lfFilter) return false;

    // --- DEFINITIVE FIX: The erroneous reset is removed. ---
    // *lfFilter = context.lf_params_snapshot; // This was the bug.

    lfFilter->settleThreshold = hfFilter->settleThreshold * 0.1;
    lfFilter->settleThreshold = constrain(lfFilter->settleThreshold, 0.001, 0.5);
    lfFilter->medianWindowSize = 15;
    lfFilter->trackResponse = 0.05;
    lfFilter->lockSmoothing = 0.005;
    lfFilter->trackAssist = 0.0001;

    LOG_AUTO_TUNE("LF Stage Optimized. Settle=%.3f", lfFilter->settleThreshold);
    return true;
}

void GuidedTuningEngine::applyResults(PBiosContext& context) {
    LOG_AUTO_TUNE("Guided Tuning results have been applied to the working_params.");
}