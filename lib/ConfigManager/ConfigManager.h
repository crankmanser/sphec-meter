// File Path: /lib/ConfigManager/ConfigManager.h
// MODIFIED FILE

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <ArduinoJson.h>
#include <FaultHandler.h>
#include "FilterManager.h"
#include "SdManager.h"

class ConfigManager {
public:
    ConfigManager();
    bool begin(FaultHandler& faultHandler, SdManager& sdManager);

    /**
     * @brief Saves the settings of a FilterManager to a JSON file.
     * @param filter The FilterManager instance to save.
     * @param filterName The base name for the config file (e.g., "ph_filter").
     * @param is_saved_state If true, appends "_saved" to the filename for the user's backup.
     * @return True if saving was successful, false otherwise.
     */
    bool saveFilterSettings(FilterManager& filter, const char* filterName, bool is_saved_state = false);

    /**
     * @brief Loads the settings for a FilterManager from a JSON file.
     * @param filter The FilterManager instance to load into.
     * @param filterName The base name for the config file (e.g., "ph_filter").
     * @param is_saved_state If true, loads from the user's "_saved" backup file.
     * @return True if loading was successful, false otherwise.
     */
    bool loadFilterSettings(FilterManager& filter, const char* filterName, bool is_saved_state = false);

private:
    FaultHandler* _faultHandler;
    SdManager* _sdManager;
    bool _initialized;
};

#endif // CONFIG_MANAGER_H