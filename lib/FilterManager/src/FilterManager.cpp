// File Path: /lib/FilterManager/src/FilterManager.cpp
// MODIFIED FILE

#include "FilterManager.h"
#include "ConfigManager.h"
#include "DebugConfig.h"

FilterManager::FilterManager() :
    _faultHandler(nullptr),
    _initialized(false)
{}

/**
 * @brief Initializes the FilterManager with hardcoded defaults.
 * @version 3.1.13
 */
bool FilterManager::begin(FaultHandler& faultHandler, const char* name) {
    _faultHandler = &faultHandler;
    _name = name;
    _hfFilter.medianWindowSize = 5;
    _hfFilter.settleThreshold = 0.1;
    _hfFilter.lockSmoothing = 0.1;
    _hfFilter.trackResponse = 0.6;
    _hfFilter.trackAssist = 0.01;
    _lfFilter.medianWindowSize = 15;
    _lfFilter.settleThreshold = 0.01;
    _lfFilter.lockSmoothing = 0.005;
    _lfFilter.trackResponse = 0.05;
    _lfFilter.trackAssist = 0.0001;
    _initialized = true;
    return true;
}

double FilterManager::process(double rawVoltage) {
    if (!_initialized) {
        return rawVoltage;
    }
    LOG_FILTER_PIPELINE("FilterManager '%s' received raw value: %.4f", _name.c_str(), rawVoltage);
    double hfFiltered = _hfFilter.process(rawVoltage);
    LOG_FILTER_PIPELINE(" > HF Filter output: %.4f", hfFiltered);
    double lfFiltered = _lfFilter.process(hfFiltered);
    LOG_FILTER_PIPELINE(" > LF Filter output: %.4f", lfFiltered);
    return lfFiltered;
}

PI_Filter* FilterManager::getFilter(int index) {
    if (index == 0) return &_hfFilter;
    if (index == 1) return &_lfFilter;
    return nullptr;
}

/**
 * @brief --- NEW: Implementation of the total noise reduction KPI. ---
 * This function encapsulates the one true calculation for pipeline performance.
 * It compares the standard deviation of the initial raw signal (from the HF
 * filter's perspective) to the standard deviation of the final clean signal
 * (from the LF filter's output).
 * @version 3.1.13
 */
int FilterManager::getNoiseReductionPercentage() const {
    double raw_std = _hfFilter.getRawStandardDeviation();
    double final_std = _lfFilter.getFilteredStandardDeviation();

    if (raw_std > 1e-9) {
        double improvement = 1.0 - (final_std / raw_std);
        return (int)constrain(improvement * 100.0, 0.0, 100.0);
    }
    return 100;
}