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
        
        configManager.saveFilterSettings(*this, _name.c_str(), "default");
    }

    _initialized = true;
    return true;
}

/**
 * @brief Processes a raw voltage value through the two-stage filter pipeline.
 *
 * This is the core of the filtering engine. It implements the sequential,
 * two-stage process defined in the architectural blueprint:
 * 1. The raw signal is first passed to the High-Frequency (HF) filter.
 * 2. The *output* of the HF filter is then passed to the Low-Frequency (LF) filter.
 *
 * @param rawVoltage The raw, unfiltered voltage from the ADC.
 * @return The final, filtered voltage value.
 */
double FilterManager::process(double rawVoltage) {
    if (!_initialized) {
        return rawVoltage;
    }
    LOG_FILTER_PIPELINE("FilterManager '%s' received raw value: %.4f", _name.c_str(), rawVoltage);
    
    // Stage 1: The "Spike Scraper"
    // The raw signal is processed by the HF filter to remove fast, sharp noise.
    double hfFiltered = _hfFilter.process(rawVoltage);
    LOG_FILTER_PIPELINE(" > HF Filter output: %.4f", hfFiltered);

    // --- DEFINITIVE FIX: The LF filter now processes the output of the HF filter. ---
    // This corrects the pipeline logic, ensuring the LF "Smoothing Squeegee"
    // receives the pre-cleaned signal it is designed to work with.
    double lfFiltered = _lfFilter.process(hfFiltered);
    LOG_FILTER_PIPELINE(" > LF Filter output: %.4f", lfFiltered);
    
    return lfFiltered;
}

PI_Filter* FilterManager::getFilter(int index) {
    if (index == 0) return &_hfFilter;
    if (index == 1) return &_lfFilter;
    return nullptr;
}