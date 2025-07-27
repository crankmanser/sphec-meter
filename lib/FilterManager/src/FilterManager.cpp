// File Path: /lib/FilterManager/src/FilterManager.cpp

#include "FilterManager.h"
#include <ConfigManager.h> // Required for loading parameters

/**
 * @brief Constructor for the FilterManager.
 *
 * Initializes member variables to a safe, default state.
 * The actual filter parameters will be loaded during the begin() method.
 */
FilterManager::FilterManager() :
    _faultHandler(nullptr),
    _initialized(false)
{}

/**
 * @brief Initializes the FilterManager and loads filter parameters.
 *
 * This method will (in the future) interact with the ConfigManager to load
 * the persisted tuning parameters for both the HF and LF filters from the
 * ESP32's internal flash (NVS).
 *
 * @param faultHandler A reference to the global fault handler.
 * @param configManager A reference to the ConfigManager.
 * @return True, indicating successful initialization.
 */
bool FilterManager::begin(FaultHandler& faultHandler, ConfigManager& configManager) {
    _faultHandler = &faultHandler;

    // --- TODO: Load parameters from ConfigManager ---
    // In a future step, we will implement the logic here to call
    // methods on the configManager to retrieve the saved tuning values
    // for _hfFilter and _lfFilter.
    //
    // Example (pseudo-code):
    // _hfFilter.settleThreshold = configManager.getFloat("hf_settle_threshold", 0.1);
    // _lfFilter.lockSmoothing = configManager.getFloat("lf_lock_smoothing", 0.01);
    
    _initialized = true;
    return true;
}

/**
 * @brief Processes a raw voltage through the complete two-stage pipeline.
 *
 * This method embodies the core data flow of the filter engine. The raw
 * value is first passed to the HF filter, and the clean output of that stage
 * is then used as the input for the LF filter.
 *
 * @param rawVoltage The raw voltage from the AdcManager.
 * @return The final, clean voltage after both stages of filtering.
 */
double FilterManager::process(double rawVoltage) {
    if (!_initialized) {
        return rawVoltage; // Return the raw value if not initialized
    }

    // Stage 1: Process through the High-Frequency filter
    double hfFiltered = _hfFilter.process(rawVoltage);

    // Stage 2: Process the result of Stage 1 through the Low-Frequency filter
    double lfFiltered = _lfFilter.process(hfFiltered);

    return lfFiltered;
}

/**
 * @brief Provides direct access to the internal filter objects for tuning.
 *
 * This method acts as a gateway for the pBIOS diagnostics and tuning screens.
 * It allows the UI code to get a pointer to a specific filter stage to read
 * its statistics (like standard deviation) and to modify its public tuning
 * parameters in real-time.
 *
 * @param index 0 to get the HF filter, 1 to get the LF filter.
 * @return A pointer to the requested PI_Filter object, or nullptr.
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