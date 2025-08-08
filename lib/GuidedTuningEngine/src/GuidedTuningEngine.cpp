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

    targetHfFilter->settleThreshold = _rawStdDev * 1.5;
    targetHfFilter->settleThreshold = constrain(targetHfFilter->settleThreshold, 0.01, 1.0);
    targetHfFilter->medianWindowSize = (_peakFrequency > 150) ? 7 : 5;
    targetHfFilter->trackResponse = map_double(_peakFrequency, 10.0, 500.0, 0.4, 0.95);
    targetHfFilter->trackResponse = constrain(targetHfFilter->trackResponse, 0.4, 0.95);
    targetHfFilter->lockSmoothing = map_double(targetHfFilter->trackResponse, 0.4, 0.95, 0.2, 0.01);
    targetHfFilter->lockSmoothing = constrain(targetHfFilter->lockSmoothing, 0.01, 0.2);
    targetHfFilter->trackAssist = 0.01;

    LOG_AUTO_TUNE("HF Results: Settle=%.3f, Resp=%.2f, Smooth=%.2f", targetHfFilter->settleThreshold, targetHfFilter->trackResponse, targetHfFilter->lockSmoothing);
}

void GuidedTuningEngine::deriveLfParameters(PI_Filter* hfFilter, PI_Filter* targetLfFilter) {
    if (!hfFilter || !targetLfFilter) return;

    targetLfFilter->settleThreshold = hfFilter->settleThreshold * 0.1;
    targetLfFilter->settleThreshold = constrain(targetLfFilter->settleThreshold, 0.001, 0.5);
    targetLfFilter->medianWindowSize = 15;
    targetLfFilter->trackResponse = 0.05;
    targetLfFilter->lockSmoothing = 0.005;
    targetLfFilter->trackAssist = 0.0001;

    // --- DEFINITIVE FIX: Corrected the typo in the logging macro parameter ---
    LOG_AUTO_TUNE("LF Results: Settle=%.3f, Resp=%.2f, Smooth=%.3f", targetLfFilter->settleThreshold, targetLfFilter->trackResponse, targetLfFilter->lockSmoothing);
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
    for (int i = 0; i < GT_SAMPLE_COUNT; ++i) sum += _rawSamples[i];
    double mean = sum / GT_SAMPLE_COUNT;

    double sumSqDiff = 0.0;
    for (int i = 0; i < GT_SAMPLE_COUNT; ++i) {
        _fftReal[i] = _rawSamples[i] - mean;
        _fftImag[i] = 0.0;
        sumSqDiff += _fftReal[i] * _fftReal[i];
    }
    _rawStdDev = sqrt(sumSqDiff / GT_SAMPLE_COUNT);

    _FFT->Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    _FFT->Compute(FFT_FORWARD);
    _FFT->ComplexToMagnitude();
    _peakFrequency = _FFT->MajorPeak();
}