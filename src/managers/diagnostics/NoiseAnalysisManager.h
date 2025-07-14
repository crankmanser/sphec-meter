// src/managers/diagnostics/NoiseAnalysisManager.h
// MODIFIED FILE
#pragma once

#include "hal/ADS1118_Driver.h"
#include "hal/INA219_Driver.h"
#include "helpers/diagnostics/StatisticalAnalyzer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

enum class SensorToAnalyze {
    PH_PROBE,
    EC_PROBE,
    BUS_3V3,
    BUS_5V,
    INA219_VOLTAGE,
    INA219_CURRENT
};

class NoiseAnalysisManager {
public:
    /**
     * @brief Constructs the NoiseAnalysisManager.
     * @param adc1 Pointer to the ADS1118 driver for pH and 3.3V Bus.
     * @param adc2 Pointer to the ADS1118 driver for EC and 5V Bus.
     * @param ina219 Pointer to the INA219 driver.
     * @param sensorTaskHandle Handle to the main SensorTask.
     * @param telemetryTaskHandle Handle to the TelemetryTask.
     * @param connectivityTaskHandle Handle to the ConnectivityTask.
     */
    NoiseAnalysisManager(
        ADS1118_Driver* adc1, 
        ADS1118_Driver* adc2, 
        INA219_Driver* ina219,
        TaskHandle_t sensorTaskHandle,
        TaskHandle_t telemetryTaskHandle,
        TaskHandle_t connectivityTaskHandle
    );

    StatisticalResult runAnalysis(SensorToAnalyze sensor);

private:
    ADS1118_Driver* _adc1;
    ADS1118_Driver* _adc2;
    INA219_Driver* _ina219;
    TaskHandle_t _sensorTaskHandle;
    TaskHandle_t _telemetryTaskHandle;
    TaskHandle_t _connectivityTaskHandle;

    static constexpr int SAMPLE_COUNT = 1024;
    static constexpr int SAMPLING_RATE_HZ = 250;
    static constexpr int SAMPLING_INTERVAL_US = 1000000 / SAMPLING_RATE_HZ;

    void enterFocusedMode();
    void exitFocusedMode();
    float adcValueToVoltage(int16_t raw_value);
    float ina219Value(SensorToAnalyze sensor);
};