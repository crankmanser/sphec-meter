// File Path: /lib/GuidedTuningEngine/src/GuidedTuningEngine.cpp
// MODIFIED FILE

#include "GuidedTuningEngine.h"
#include "../../src/DebugConfig.h" // For LOG_AUTO_TUNE
#include <Arduino.h>
#include <cmath>
#include <algorithm> 
#include "freertos/FreeRTOS.h" 
#include "freertos/task.h"     

const float TUNING_LEARNING_RATE = 0.4f;

// Helper function for mapping values (used in heuristics)
double map_double(double x, double in_min, double in_max, double out_min, double out_max) {
    if (in_max - in_min == 0) return out_min;
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

GuidedTuningEngine::GuidedTuningEngine() {
    // Constructor allocates memory for FFT on the heap
    _fftReal = new double[GT_SAMPLE_COUNT];
    _fftImag = new double[GT_SAMPLE_COUNT];
    _FFT = new arduinoFFT(_fftReal, _fftImag, GT_SAMPLE_COUNT, 1000.0); // Assuming 1kHz effective sample rate
}

GuidedTuningEngine::~GuidedTuningEngine() {
    // Destructor safely frees the heap-allocated memory
    delete _FFT;
    delete[] _fftReal;
    delete[] _fftImag;
}

/**
 * @brief --- DEFINITIVE REFACTOR: Implements the full "Statistical Snapshot" algorithm ---
 * This is the master function that orchestrates the new, robust, RAM-based tuning process.
 */
bool GuidedTuningEngine::proposeSettings(PBiosContext& context, AdcManager& adcManager, SdManager& sdManager, StateManager* stateManager, AutoTuningScreen& progressScreen) {
    if (!context.selectedFilter) return false;

    // --- Stage 1: Multi-Pass RAM Capture & Averaging ---
    if (!captureSignal(context, adcManager, progressScreen)) {
        LOG_AUTO_TUNE("Failed to capture signal.");
        return false;
    }

    // --- Stage 2: Signal Analysis ---
    progressScreen.setProgress(30, "Analyzing Noise Fingerprint...");
    analyzeSignal(context, context.captured_samples);
    
    // --- Stage 3: HF Optimization ---
    progressScreen.setProgress(50, "Optimizing HF Stage...");
    deriveHfParameters(context);

    // --- Stage 4: LF Optimization ---
    progressScreen.setProgress(70, "Optimizing LF Stage...");
    deriveLfParameters(context); // Now uses the same rich dataset

    LOG_AUTO_TUNE("Guided Tuning complete. Refinements applied.");
    return true;
}

/**
 * @brief --- DEFINITIVE IMPLEMENTATION: The "Statistical Snapshot" capture method ---
 * This function performs three separate, short captures directly into RAM and
 * averages them to create a high-confidence, statistically robust signal profile.
 * It is fast, safe, and completely avoids all SD card interaction.
 */
bool GuidedTuningEngine::captureSignal(PBiosContext& context, AdcManager& adcManager, AutoTuningScreen& progressScreen) {
    const int num_captures = 3;
    const int samples_per_capture = GT_SAMPLE_COUNT;
    std::vector<double> averaged_samples(samples_per_capture, 0.0);

    for (int c = 0; c < num_captures; ++c) {
        char progress_label[40];
        snprintf(progress_label, sizeof(progress_label), "Sampling Pass (%d/%d)...", c + 1, num_captures);
        progressScreen.setProgress(10 * (c + 1), progress_label);
        
        for (int i = 0; i < samples_per_capture; ++i) {
            // We use the safe, mutex-protected getVoltage() here as this is a short, fast capture.
            double voltage = adcManager.getVoltage(context.selectedAdcIndex, context.selectedAdcInput);
            if(isnan(voltage)) voltage = 0;
            averaged_samples[i] += voltage;
            vTaskDelay(pdMS_TO_TICKS(2)); // Corresponds to ~500Hz, well above our 44Hz Nyquist rate
        }
    }

    // Finalize the average
    for (int i = 0; i < samples_per_capture; ++i) {
        averaged_samples[i] /= num_captures;
    }
    
    context.captured_samples = averaged_samples;
    
    bool success = (context.captured_samples.size() == samples_per_capture);
    LOG_AUTO_TUNE("RAM Signal capture complete. Success: %d", success);
    return success;
}


/**
 * @brief --- REMOVED: The problematic disk-based capture is gone. ---
 * The captureLongSignalForLF function has been completely deleted.
 */


void GuidedTuningEngine::analyzeSignal(PBiosContext& context, const std::vector<double>& signal_to_analyze) {
    if (signal_to_analyze.empty()) return;
    size_t sample_count = signal_to_analyze.size();

    double sum = 0.0;
    for (const auto& sample : signal_to_analyze) sum += sample;
    double mean = sum / sample_count;

    double sumSqDiff = 0.0;
    for (int i = 0; i < std::min((int)sample_count, GT_SAMPLE_COUNT); ++i) {
        _fftReal[i] = signal_to_analyze[i] - mean;
        _fftImag[i] = 0.0;
        sumSqDiff += _fftReal[i] * _fftReal[i];
    }
    context.raw_std_dev = sqrt(sumSqDiff / std::min((int)sample_count, GT_SAMPLE_COUNT));

    double min_val = *std::min_element(signal_to_analyze.begin(), signal_to_analyze.end());
    double max_val = *std::max_element(signal_to_analyze.begin(), signal_to_analyze.end());
    context.pk_pk_amplitude = max_val - min_val;

    _FFT->Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    _FFT->Compute(FFT_FORWARD);
    _FFT->ComplexToMagnitude();
    context.peak_frequency = _FFT->MajorPeak();
    
    LOG_AUTO_TUNE("Analysis: StdDev=%.3f, Pk-Pk=%.3f, Peak Freq=%.1f Hz", 
        context.raw_std_dev, context.pk_pk_amplitude, context.peak_frequency);
}

void GuidedTuningEngine::deriveHfParameters(PBiosContext& context) {
    PI_Filter* hfFilter = context.selectedFilter->getFilter(0);
    if (!hfFilter) return;

    PI_Filter idealParams;

    // --- Heuristic 1: Weighted average for Settle Threshold ---
    double ideal_settle = (context.raw_std_dev * 0.7) + (context.pk_pk_amplitude * 0.3);
    idealParams.settleThreshold = ideal_settle;

    // --- Heuristic 2: Inversely linked smoothing and response based on noise power ---
    if (context.pk_pk_amplitude > 25.0) {
        LOG_AUTO_TUNE("HF Strategy: Aggressive Spike Scraper");
        idealParams.trackResponse = 0.9;
        idealParams.lockSmoothing = 0.02;
    } else {
        LOG_AUTO_TUNE("HF Strategy: Gentle Smoother");
        idealParams.trackResponse = 0.6;
        idealParams.lockSmoothing = 0.1;
    }
    
    // --- Heuristic 3: Constrain and set static params ---
    idealParams.settleThreshold = constrain(idealParams.settleThreshold, 0.01, 25.0);
    idealParams.medianWindowSize = (context.peak_frequency > 150) ? 7 : 5;
    idealParams.trackAssist = 0.01;

    applyRefinement(hfFilter, idealParams);

    LOG_AUTO_TUNE("HF Stage Refined. Target Settle=%.3f, Current Settle=%.3f", 
        idealParams.settleThreshold, hfFilter->settleThreshold);
}

/**
 * @brief --- DEFINITIVE REFACTOR: The LF stage now uses the same rich dataset ---
 * This function now uses the same high-quality "noise fingerprint" from the RAM
 * capture to derive the LF parameters. It no longer requires a separate, slow
 * disk-based capture.
 */
void GuidedTuningEngine::deriveLfParameters(PBiosContext& context) {
    PI_Filter* lfFilter = context.selectedFilter->getFilter(1);
    if (!lfFilter) return;

    PI_Filter idealParams;

    // --- Heuristic 1: Settle Threshold is proportional to Pk-Pk amplitude ---
    // This targets the slow "wobble" of the signal.
    idealParams.settleThreshold = context.pk_pk_amplitude * 0.5;
    idealParams.settleThreshold = constrain(idealParams.settleThreshold, 5.0, 50.0);
    
    // --- Heuristic 2: Use fixed, aggressive smoothing for the LF stage ---
    idealParams.trackResponse = 0.05;
    idealParams.lockSmoothing = 0.005;
    idealParams.medianWindowSize = 15;
    idealParams.trackAssist = 0.0001;
    
    applyRefinement(lfFilter, idealParams);
    
    LOG_AUTO_TUNE("LF Stage Refined. Target Settle=%.3f, Current Settle=%.3f", 
        idealParams.settleThreshold, lfFilter->settleThreshold);
}


void GuidedTuningEngine::applyRefinement(PI_Filter* currentFilter, const PI_Filter& idealParams) {
    if (!currentFilter) return;

    // The "Iterative Refinement" step: move the current parameters part of the way
    // towards the ideal target. This creates a stateful, converging tune.
    currentFilter->settleThreshold += (idealParams.settleThreshold - currentFilter->settleThreshold) * TUNING_LEARNING_RATE;
    currentFilter->lockSmoothing += (idealParams.lockSmoothing - currentFilter->lockSmoothing) * TUNING_LEARNING_RATE;
    currentFilter->trackResponse += (idealParams.trackResponse - currentFilter->trackResponse) * TUNING_LEARNING_RATE;
    currentFilter->trackAssist += (idealParams.trackAssist - currentFilter->trackAssist) * TUNING_LEARNING_RATE;
    currentFilter->medianWindowSize = idealParams.medianWindowSize;
}