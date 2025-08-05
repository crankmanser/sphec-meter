// File Path: /lib/FilterManager/src/FilterManager.cpp
// MODIFIED FILE

#include "FilterManager.h"
#include <ConfigManager.h> 

/**
 * @brief Constructor for the FilterManager.
 */
FilterManager::FilterManager() :
    _faultHandler(nullptr),
    _initialized(false)
{}

/**
 * @brief --- DEFINITIVE FIX: Specializes the HF and LF filters for their domains. ---
 */
bool FilterManager::begin(FaultHandler& faultHandler, ConfigManager& configManager) {
    _faultHandler = &faultHandler;

    // --- HF Filter ("Spike Scraper"): Specialized for fast spike rejection ---
    _hfFilter.medianWindowSize = 5;      // Small window to catch fast spikes
    _hfFilter.settleThreshold = 0.1;
    _hfFilter.lockSmoothing = 0.1;     // Gentle smoothing
    _hfFilter.trackResponse = 0.6;     // Reasonably fast tracking
    _hfFilter.trackAssist = 0.01;
    
    // --- LF Filter ("Smoothing Squeegee"): Specialized for long-term drift ---
    _lfFilter.medianWindowSize = 15;     // Large window to ignore outliers
    _lfFilter.settleThreshold = 0.01;  // High sensitivity to lock in
    _lfFilter.lockSmoothing = 0.005;   // Extremely heavy smoothing
    _lfFilter.trackResponse = 0.05;    // Very slow tracking to flatten waves
    _lfFilter.trackAssist = 0.0001;

    _initialized = true;
    return true;
}

/**
 * @brief Processes a raw voltage through the complete two-stage pipeline.
 */
double FilterManager::process(double rawVoltage) {
    if (!_initialized) {
        return rawVoltage;
    }
    // --- DEFINITIVE FIX: Corrected the variable name from rawValue to rawVoltage ---
    double hfFiltered = _hfFilter.process(rawVoltage);
    double lfFiltered = _lfFilter.process(hfFiltered);
    return lfFiltered;
}

/**
 * @brief Provides direct access to the internal filter objects for tuning.
 */
PI_Filter* FilterManager::getFilter(int index) {
    if (index == 0) {
        return &_hfFilter;
    }
    if (index == 1) {
        return &_lfFilter;
    }
    return nullptr;
}