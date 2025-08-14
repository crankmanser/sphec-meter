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
 */
class FilterManager {
public:
    FilterManager();

    /**
     * @brief --- DEFINITIVE REFACTOR: Initializes the FilterManager with hardcoded defaults. ---
     * This method no longer accepts a ConfigManager and does not perform any file I/O.
     * It simply initializes the filter parameters to a safe, default state. The responsibility
     * of loading the configuration from the SD card is now handled by a centralized
     * "provisioner" function in main.cpp, which runs after all hardware is stable,
     * thus eliminating the initialization race condition.
     *
     * @param faultHandler A reference to the global fault handler.
     * @param name The unique name for this filter instance (e.g., "ph_filter").
     * @return True if initialization is successful, false otherwise.
     */
    bool begin(FaultHandler& faultHandler, const char* name);

    double process(double rawVoltage);
    PI_Filter* getFilter(int index);

private:
    FaultHandler* _faultHandler;
    bool _initialized;
    std::string _name; // Name for config file

    PI_Filter _hfFilter;
    PI_Filter _lfFilter;
};

#endif // FILTER_MANAGER_H