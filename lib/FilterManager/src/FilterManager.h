// File Path: /lib/FilterManager/src/FilterManager.h
// MODIFIED FILE

#ifndef FILTER_MANAGER_H
#define FILTER_MANAGER_H

#include <FaultHandler.h>
#include "PI_Filter.h"
#include <string>

// Forward declaration to avoid circular dependency
class ConfigManager;

/**
 * @class FilterManager
 * @brief Manages the two-stage (HF/LF) filtering pipeline for a single signal source.
 * @version 3.1.13
 */
class FilterManager {
public:
    FilterManager();
    bool begin(FaultHandler& faultHandler, const char* name);
    double process(double rawVoltage);
    PI_Filter* getFilter(int index);

    /**
     * @brief --- NEW: Gets the total noise reduction across the entire pipeline. ---
     * This is the definitive metric for filter performance, comparing the raw
     * input noise to the final, clean output.
     * @return The total noise reduction as a percentage (0-100%).
     */
    int getNoiseReductionPercentage() const;

private:
    FaultHandler* _faultHandler;
    bool _initialized;
    std::string _name; // Name for config file

    PI_Filter _hfFilter;
    PI_Filter _lfFilter;
};

#endif // FILTER_MANAGER_H