// File Path: /lib/FilterManager/src/FilterManager.cpp
// MODIFIED FILE

#include "FilterManager.h"
#include "ConfigManager.h" 
// --- FIX: Use a direct relative path to find the header in the src directory ---
#include "../../src/DebugConfig.h"

FilterManager::FilterManager() :
    _faultHandler(nullptr),
    _initialized(false)
{}

bool FilterManager::begin(FaultHandler& faultHandler, ConfigManager& configManager, const char* name) {
    _faultHandler = &faultHandler;
    _name = name;

    // --- NEW: Load settings from ConfigManager ---
    // Try to load the settings from the file on the SD card.
    if (!configManager.loadFilterSettings(*this, _name.c_str())) {
        LOG_STORAGE("No config file for '%s', creating with defaults.", _name.c_str());
        // If loading fails (e.g., file not found), set default values first.
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
        
        // Then, save these default settings to create the file for next time.
        configManager.saveFilterSettings(*this, _name.c_str());
    }

    _initialized = true;
    return true;
}

double FilterManager::process(double rawVoltage) {
    if (!_initialized) {
        return rawVoltage;
    }
    double hfFiltered = _hfFilter.process(rawVoltage);
    double lfFiltered = _lfFilter.process(hfFiltered);
    return lfFiltered;
}

PI_Filter* FilterManager::getFilter(int index) {
    if (index == 0) return &_hfFilter;
    if (index == 1) return &_lfFilter;
    return nullptr;
}