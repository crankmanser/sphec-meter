// src/managers/diagnostics/NoiseAnalysisManager.cpp
// MODIFIED FILE
#include "NoiseAnalysisManager.h"
#include "DebugMacros.h"
#include <vector>

const float VOLTAGE_DIVIDER_COMPENSATION_N = 2.0f;
const float ADC_FSR_PROBES_N = 4.096f;

NoiseAnalysisManager::NoiseAnalysisManager(
    ADS1118_Driver* adc1,
    ADS1118_Driver* adc2,
    INA219_Driver* ina219,
    TaskHandle_t sensorTaskHandle,
    TaskHandle_t telemetryTaskHandle,
    TaskHandle_t connectivityTaskHandle) :
    _adc1(adc1),
    _adc2(adc2),
    _ina219(ina219),
    _sensorTaskHandle(sensorTaskHandle),
    _telemetryTaskHandle(telemetryTaskHandle),
    _connectivityTaskHandle(connectivityTaskHandle)
{}

bool NoiseAnalysisManager::performAnalysis(SensorToAnalyze sensor) {
    LOG_MANAGER("--- Entering Focused Analysis Mode for %d ---\n", (int)sensor);
    enterFocusedMode();

    if (!_adc1 || !_adc2 || !_ina219) {
        LOG_MAIN("[NAM_ERROR] A required driver is null. Aborting analysis.\n");
        exitFocusedMode();
        return false;
    }

    std::vector<float>* samples = new std::vector<float>();
    samples->reserve(SAMPLE_COUNT);

    LOG_MANAGER("Capturing %d samples at %d Hz...\n", SAMPLE_COUNT, SAMPLING_RATE_HZ);
    for (int i = 0; i < SAMPLE_COUNT; ++i) {
        float value = 0.0f;
        switch(sensor) {
            case SensorToAnalyze::PH_PROBE:      value = adcValueToVoltage(_adc1->readDifferential_0_1()); break;
            case SensorToAnalyze::EC_PROBE:      value = adcValueToVoltage(_adc2->readDifferential_0_1()); break;
            case SensorToAnalyze::BUS_3V3:       value = adcValueToVoltage(_adc1->readSingleEnded_2());    break;
            case SensorToAnalyze::BUS_5V:        value = adcValueToVoltage(_adc2->readSingleEnded_2());    break;
            case SensorToAnalyze::INA219_VOLTAGE: value = _ina219->getBusVoltage();                        break;
            case SensorToAnalyze::INA219_CURRENT: value = _ina219->getCurrent_mA();                      break;
        }
        samples->push_back(value);
        
        // <<< FIX: Use a non-blocking delay to yield control to the scheduler >>>
        // We run the loop every ~1ms. A delay of 1 tick is perfect here.
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    LOG_MANAGER("Sample capture complete.\n");

    exitFocusedMode();
    LOG_MANAGER("--- Exited Focused Analysis Mode ---\n");

    LOG_MANAGER("Running statistical analysis...\n");
    _last_stat_result = StatisticalAnalyzer::analyze(*samples);

    LOG_MANAGER("Running FFT analysis...\n");
    _last_fft_result = FftAnalyzer::analyze(*samples, SAMPLING_RATE_HZ);

    LOG_MANAGER("Deriving recommended filter tuning...\n");
    _recommended_tuning = AutoTuner::tunePiFilter(_last_stat_result);
    
    delete samples;
    samples = nullptr;

    if (_last_stat_result.is_valid) {
        LOG_MANAGER("--- Noise Analysis Results ---");
        LOG_MANAGER("  Mean: %.4f", _last_stat_result.mean);
        LOG_MANAGER("  Std. Dev.: %.6f", _last_stat_result.std_dev);
        if(_last_fft_result.is_valid) {
            LOG_MANAGER("  Dominant Freq: %.2f Hz", _last_fft_result.dominant_frequency);
        }
        LOG_MANAGER("-----------------------------\n");
        return true;
    } else {
        LOG_MAIN("[NAM_ERROR] Analysis failed, not enough data.\n");
        return false;
    }
}

// ... (Rest of the file is unchanged) ...
StatisticalResult NoiseAnalysisManager::getStatisticalResult() const { return _last_stat_result; }
FftResult NoiseAnalysisManager::getFftResult() const { return _last_fft_result; }
PIFilter::Tuni_t NoiseAnalysisManager::getRecommendedTuning() const { return _recommended_tuning; }


void NoiseAnalysisManager::enterFocusedMode() {
    LOG_MANAGER("Suspending background tasks...\n");
    if (_sensorTaskHandle) vTaskSuspend(_sensorTaskHandle);
    if (_telemetryTaskHandle) vTaskSuspend(_telemetryTaskHandle);
    if (_connectivityTaskHandle) vTaskSuspend(_connectivityTaskHandle);
}

void NoiseAnalysisManager::exitFocusedMode() {
    LOG_MANAGER("Resuming background tasks...\n");
    if (_sensorTaskHandle) vTaskResume(_sensorTaskHandle);
    if (_telemetryTaskHandle) vTaskResume(_telemetryTaskHandle);
    if (_connectivityTaskHandle) vTaskResume(_connectivityTaskHandle);
}

float NoiseAnalysisManager::adcValueToVoltage(int16_t raw_value) {
    float voltage = (static_cast<float>(raw_value) / 32767.0f) * ADC_FSR_PROBES_N;
    // Don't apply voltage divider compensation to bus voltage measurements
    if (raw_value == _adc1->readSingleEnded_2() || raw_value == _adc2->readSingleEnded_2()) {
        return voltage;
    }
    return voltage * VOLTAGE_DIVIDER_COMPENSATION_N;
}