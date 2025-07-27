// File Path: /lib/FilterManager/src/FilterManager.h

#ifndef FILTER_MANAGER_H
#define FILTER_MANAGER_H

#include <FaultHandler.h>
#include "PI_Filter.h" // Include the building block

// Forward declaration to avoid circular dependencies
class ConfigManager;

/**
 * @class FilterManager
 * @brief Manages the two-stage filtering pipeline for sensor data.
 *
 * As per the architectural blueprint, this cabinet is responsible for processing
 * raw sensor voltages through a sequential, two-stage filtering process:
 * 1. High-Frequency (HF) Filter: Targets short-term electrical noise.
 * 2. Low-Frequency (LF) Filter: Smooths out longer-term signal drift.
 *
 * It provides a single interface for the rest of the application to get clean
 * data and for the pBIOS to access and tune the filter parameters.
 */
class FilterManager {
public:
    /**
     * @brief Constructor for the FilterManager.
     */
    FilterManager();

    /**
     * @brief Initializes the FilterManager.
     * @param faultHandler A reference to the global fault handler.
     * @param configManager A reference to the ConfigManager to load parameters.
     * @return True if initialization is successful, false otherwise.
     */
    bool begin(FaultHandler& faultHandler, ConfigManager& configManager);

    /**
     * @brief Processes a raw voltage through the complete two-stage pipeline.
     * @param rawVoltage The raw voltage from the AdcManager.
     * @return The final, clean voltage after both HF and LF filtering.
     */
    double process(double rawVoltage);

    /**
     * @brief Retrieves a pointer to one of the internal filter objects.
     * This is used by the pBIOS tuning screen to directly access and modify
     * the parameters of a specific filter stage.
     * @param index 0 for the HF filter, 1 for the LF filter.
     * @return A pointer to the PI_Filter object, or nullptr if the index is invalid.
     */
    PI_Filter* getFilter(int index);

private:
    FaultHandler* _faultHandler;
    bool _initialized;

    // The two instances of our filter building block, one for each stage.
    PI_Filter _hfFilter; // High-Frequency Filter
    PI_Filter _lfFilter; // Low-Frequency Filter
};

#endif // FILTER_MANAGER_H