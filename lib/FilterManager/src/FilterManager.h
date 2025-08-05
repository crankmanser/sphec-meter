// File Path: /lib/FilterManager/src/FilterManager.h
// MODIFIED FILE

#ifndef FILTER_MANAGER_H
#define FILTER_MANAGER_H

#include <FaultHandler.h>
#include "PI_Filter.h"
#include <string>

class ConfigManager;

class FilterManager {
public:
    FilterManager();

    /**
     * @brief Initializes the FilterManager and loads its settings.
     * @param faultHandler A reference to the global fault handler.
     * @param configManager A reference to the ConfigManager to load/save settings.
     * @param name The unique name for this filter instance (e.g., "ph_filter").
     * @return True if initialization is successful, false otherwise.
     */
    bool begin(FaultHandler& faultHandler, ConfigManager& configManager, const char* name);

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