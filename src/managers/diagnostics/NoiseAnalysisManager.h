// src/managers/diagnostics/NoiseAnalysisManager.h
// MODIFIED FILE
#pragma once

#include "hal/ADS1118_Driver.h"
#include "hal/INA219_Driver.h"
#include "helpers/diagnostics/StatisticalAnalyzer.h"
#include "helpers/diagnostics/FftAnalyzer.h"
#include "helpers/diagnostics/AutoTuner.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// <<< FIX: Forward-declare the manager class >>>
class NoiseAnalysisManager;

enum class SensorToAnalyze {
    PH_PROBE,
    EC_PROBE,
    BUS_3V3,
    BUS_5V,
    INA219_VOLTAGE,
    INA219_CURRENT
};

// <<< FIX: The struct is now defined before the class that uses it. >>>
struct NoiseAnalysisParams {
    NoiseAnalysisManager* manager;
    SensorToAnalyze sensor;
    volatile bool* completion_flag;
};

class NoiseAnalysisManager {
public:
    NoiseAnalysisManager(
        ADS1118_Driver* adc1,
        ADS1118_Driver* adc2,
        INA219_Driver* ina219,
        TaskHandle_t sensorTaskHandle,
        TaskHandle_t telemetryTaskHandle,
        TaskHandle_t connectivityTaskHandle
    );

    // This is the core logic, now public to be called by our task
    bool performAnalysis(SensorToAnalyze sensor);

    // --- Public Getters for Analysis Results ---
    StatisticalResult getStatisticalResult() const;
    FftResult getFftResult() const;
    PIFilter::Tuni_t getRecommendedTuning() const;

private:
    // Friend declaration to allow our task to access private members
    friend void noiseAnalysisTask(void* pvParameters);

    ADS1118_Driver* _adc1;
    ADS1118_Driver* _adc2;
    INA219_Driver* _ina219;
    TaskHandle_t _sensorTaskHandle;
    TaskHandle_t _telemetryTaskHandle;
    TaskHandle_t _connectivityTaskHandle;

    // --- Member variables to store results ---
    StatisticalResult _last_stat_result;
    FftResult _last_fft_result;
    PIFilter::Tuni_t _recommended_tuning;


    static constexpr int SAMPLE_COUNT = 1024;
    static constexpr int SAMPLING_RATE_HZ = 860; // Max rate of ADS1118
    static constexpr int SAMPLING_INTERVAL_US = 1000000 / SAMPLING_RATE_HZ;

    void enterFocusedMode();
    void exitFocusedMode();
    float adcValueToVoltage(int16_t raw_value);
};