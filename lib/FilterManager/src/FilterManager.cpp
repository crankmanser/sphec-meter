// File Path: /lib/FilterManager/src/FilterManager.cpp
// MODIFIED FILE

#include "FilterManager.h"
#include "ConfigManager.h" 
#include "DebugConfig.h"

FilterManager::FilterManager() :
    _faultHandler(nullptr),
    _initialized(false)
{}

bool FilterManager::begin(FaultHandler& faultHandler, ConfigManager& configManager, const char* name) {
    _faultHandler = &faultHandler;
    _name = name;

    if (!configManager.loadFilterSettings(*this, _name.c_str())) {
        LOG_STORAGE("No default config file for '%s', creating with defaults.", _name.c_str());
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
        
        // --- DEFINITIVE FIX: Aligns with the new save logic. ---
        // This correctly saves the initial parameters to the primary operational file (e.g., "ph_filter.json").
        configManager.saveFilterSettings(*this, _name.c_str(), "default");
    }

    _initialized = true;
    return true;
}

double FilterManager::process(double rawVoltage) {
    if (!_initialized) {
        return rawVoltage;
    }
    LOG_FILTER("FilterManager '%s' received raw value: %.4f", _name.c_str(), rawVoltage);
    
    double hfFiltered = _hfFilter.process(rawVoltage);
    LOG_FILTER(" > HF Filter output: %.4f", hfFiltered);

    double lfFiltered = _lfFilter.process(hfFiltered);
    LOG_FILTER(" > LF Filter output: %.4f", lfFiltered);
    
    return lfFiltered;
}

PI_Filter* FilterManager::getFilter(int index) {
    if (index == 0) return &_hfFilter;
    if (index == 1) return &_lfFilter;
    return nullptr;
}