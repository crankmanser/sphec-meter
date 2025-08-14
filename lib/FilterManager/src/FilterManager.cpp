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
 * @brief --- DEFINITIVE REFACTOR: Initializes the FilterManager with hardcoded defaults. ---
 * This method no longer performs any file I/O, which is the key to resolving
 * the initialization race condition. It simply sets the filter parameters to a
 * safe, default state. The responsibility of loading the configuration from
 * the SD card is now handled by a centralized function in main.cpp.
 */
bool FilterManager::begin(FaultHandler& faultHandler, const char* name) {
    _faultHandler = &faultHandler;
    _name = name;

    // Set hardcoded default values. These will be used until the stable,
    // centralized "provisioner" in main.cpp has a chance to load the
    // real settings from the SD card.
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