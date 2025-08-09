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
const char* LF_CAPTURE_FILE_PATH = "/config/%s_capture.bin"; 

// ... (map_double, constructor, destructor, proposeSettings, captureSignal, analyzeSignal, and deriveHfParameters are unchanged) ...
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

bool GuidedTuningEngine::proposeSettings(PBiosContext& context, AdcManager& adcManager, SdManager& sdManager, StateManager* stateManager, AutoTuningScreen& progressScreen) {
    if (!context.selectedFilter) return false;

    if (!captureSignal(context, adcManager, progressScreen)) {
        LOG_AUTO_TUNE("Failed to capture signal.");
        return false;
    }

    progressScreen.setProgress(30, "Analyzing HF Signal...");
    analyzeSignal(context, context.captured_samples);
    
    progressScreen.setProgress(40, "Optimizing HF Stage...");
    deriveHfParameters(context);

    progressScreen.setProgress(50, "Optimizing LF Stage...");
    deriveLfParameters(context, adcManager, sdManager, stateManager);

    LOG_AUTO_TUNE("Guided Tuning complete. Refinements applied.");
    return true;
}

bool GuidedTuningEngine::captureSignal(PBiosContext& context, AdcManager& adcManager, AutoTuningScreen& progressScreen) {
    progressScreen.setProgress(10, "Listening to HF Signal...");
    context.captured_samples.clear();
    context.captured_samples.reserve(GT_SAMPLE_COUNT);
    
    const int num_captures = 3;
    std::vector<double> temp_samples(GT_SAMPLE_COUNT, 0.0);

    for (int c = 0; c < num_captures; ++c) {
        for (int i = 0; i < GT_SAMPLE_COUNT; ++i) {
            double voltage = adcManager.getVoltage(context.selectedAdcIndex, context.selectedAdcInput);
            if(isnan(voltage)) voltage = 0;
            temp_samples[i] += voltage;
            delayMicroseconds(500);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    for (int i = 0; i < GT_SAMPLE_COUNT; ++i) {
        context.captured_samples.push_back(temp_samples[i] / num_captures);
    }
    
    bool success = (context.captured_samples.size() == GT_SAMPLE_COUNT);
    LOG_AUTO_TUNE("HF Signal capture complete. Success: %d", success);
    return success;
}

bool GuidedTuningEngine::captureLongSignalForLF(PBiosContext& context, AdcManager& adcManager, SdManager& sdManager, AutoTuningScreen& progressScreen, const char* filepath) {
    const int num_captures = 3;
    const int capture_duration_s = 10;
    const int samples_per_s = 200;
    const int total_samples_per_capture = capture_duration_s * samples_per_s;

    FsFile captureFile = sdManager.open(filepath, FILE_WRITE);
    if (!captureFile) {
        LOG_AUTO_TUNE("Error: Could not create capture file: %s", filepath);
        return false;
    }

    for (int c = 0; c < num_captures; ++c) {
        char progress_label[40];
        snprintf(progress_label, sizeof(progress_label), "Analyzing LF Drift (%d/%d)...", c + 1, num_captures);
        LOG_AUTO_TUNE("%s", progress_label);
        
        for (int i = 0; i < total_samples_per_capture; ++i) {
            double voltage = adcManager.getVoltage_noLock(context.selectedAdcIndex, context.selectedAdcInput);
            if (isnan(voltage)) voltage = 0;
            
            captureFile.write(reinterpret_cast<uint8_t*>(&voltage), sizeof(double));

            int progress = 50 + static_cast<int>(((c * total_samples_per_capture + i) / static_cast<float>(num_captures * total_samples_per_capture)) * 30.0);
            progressScreen.setProgress(progress, progress_label);

            delay(5);
        }
    }

    captureFile.close();
    LOG_AUTO_TUNE("Long-duration capture saved to: %s", filepath);
    return true;
}


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

    double ideal_settle = (context.raw_std_dev * 0.7) + (context.pk_pk_amplitude * 0.3);
    idealParams.settleThreshold = ideal_settle;

    if (context.pk_pk_amplitude > 25.0) {
        LOG_AUTO_TUNE("HF Strategy: Aggressive Spike Scraper");
        idealParams.trackResponse = 0.9;
        idealParams.lockSmoothing = 0.02;
    } else {
        LOG_AUTO_TUNE("HF Strategy: Gentle Smoother");
        idealParams.trackResponse = 0.6;
        idealParams.lockSmoothing = 0.1;
    }
    
    idealParams.settleThreshold = constrain(idealParams.settleThreshold, 0.01, 25.0);
    idealParams.medianWindowSize = (context.peak_frequency > 150) ? 7 : 5;
    idealParams.trackAssist = 0.01;

    applyRefinement(hfFilter, idealParams);

    LOG_AUTO_TUNE("HF Stage Refined. Target Settle=%.3f, Current Settle=%.3f", 
        idealParams.settleThreshold, hfFilter->settleThreshold);
}

void GuidedTuningEngine::deriveLfParameters(PBiosContext& context, AdcManager& adcManager, SdManager& sdManager, StateManager* stateManager) {
    PI_Filter* hfFilter = context.selectedFilter->getFilter(0);
    PI_Filter* lfFilter = context.selectedFilter->getFilter(1);
    if (!hfFilter || !lfFilter) return;

    AutoTuningScreen* tuneScreen = static_cast<AutoTuningScreen*>(stateManager->getScreen(ScreenState::AUTO_TUNE_RUNNING));
    if (!tuneScreen) return;
    
    char capture_filepath[64];
    snprintf(capture_filepath, sizeof(capture_filepath), LF_CAPTURE_FILE_PATH, context.selectedFilterName.c_str());
    
    if (sdManager.takeMutex()) {
        adcManager.getVoltage_noLock(context.selectedAdcIndex, context.selectedAdcInput);

        if (!captureLongSignalForLF(context, adcManager, sdManager, *tuneScreen, capture_filepath)) {
            LOG_AUTO_TUNE("Error: Failed to capture long signal for LF analysis.");
            sdManager.giveMutex(); 
            return;
        }

        FsFile captureFile = sdManager.open(capture_filepath, FILE_READ);
        if (!captureFile) {
            LOG_AUTO_TUNE("Error: Could not open capture file for reading: %s", capture_filepath);
            sdManager.remove(capture_filepath);
            sdManager.giveMutex();
            return;
        }

        std::vector<double> long_signal_samples;
        long_signal_samples.resize(captureFile.size() / sizeof(double));
        captureFile.read(long_signal_samples.data(), captureFile.size());
        captureFile.close();
        
        sdManager.remove(capture_filepath);
        sdManager.giveMutex();

        analyzeSignal(context, long_signal_samples);

        PI_Filter idealParams;
        idealParams.settleThreshold = context.pk_pk_amplitude * 0.5;
        idealParams.settleThreshold = constrain(idealParams.settleThreshold, 5.0, 50.0);
        idealParams.trackResponse = 0.05;
        idealParams.lockSmoothing = 0.005;
        idealParams.medianWindowSize = 15;
        idealParams.trackAssist = 0.0001;
        
        applyRefinement(lfFilter, idealParams);
        
        LOG_AUTO_TUNE("LF Stage Refined. Target Settle=%.3f, Current Settle=%.3f", 
            idealParams.settleThreshold, lfFilter->settleThreshold);
    }
}


void GuidedTuningEngine::applyRefinement(PI_Filter* currentFilter, const PI_Filter& idealParams) {
    if (!currentFilter) return;

    currentFilter->settleThreshold += (idealParams.settleThreshold - currentFilter->settleThreshold) * TUNING_LEARNING_RATE;
    currentFilter->lockSmoothing += (idealParams.lockSmoothing - currentFilter->lockSmoothing) * TUNING_LEARNING_RATE;
    currentFilter->trackResponse += (idealParams.trackResponse - currentFilter->trackResponse) * TUNING_LEARNING_RATE;
    currentFilter->trackAssist += (idealParams.trackAssist - currentFilter->trackAssist) * TUNING_LEARNING_RATE;
    currentFilter->medianWindowSize = idealParams.medianWindowSize;
}