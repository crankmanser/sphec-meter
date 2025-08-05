// File Path: /lib/ConfigManager/ConfigManager.h
// MODIFIED FILE

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <ArduinoJson.h>
#include <FaultHandler.h>
#include "FilterManager.h" // Now needs the full definition
#include "SdManager.h"     // Needs the SdManager for file operations

class ConfigManager {
public:
    ConfigManager();

    /**
     * @brief Initializes the ConfigManager.
     * @param faultHandler A reference to the global fault handler.
     * @param sdManager A reference to the SdManager for file operations.
     * @return True if initialization is successful, false otherwise.
     */
    bool begin(FaultHandler& faultHandler, SdManager& sdManager);

    /**
     * @brief Saves the settings of a FilterManager to a JSON file.
     * @param filter The FilterManager instance to save.
     * @param filterName The base name for the config file (e.g., "ph_filter").
     * @return True if saving was successful, false otherwise.
     */
    bool saveFilterSettings(FilterManager& filter, const char* filterName);

    /**
     * @brief Loads the settings for a FilterManager from a JSON file.
     * @param filter The FilterManager instance to load into.
     * @param filterName The base name for the config file (e.g., "ph_filter").
     * @return True if loading was successful, false otherwise.
     */
    bool loadFilterSettings(FilterManager& filter, const char* filterName);

private:
    FaultHandler* _faultHandler;
    SdManager* _sdManager; // Pointer to the SD card manager
    bool _initialized;
};

#endif // CONFIG_MANAGER_H