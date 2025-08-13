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
     * @brief --- DEFINITIVE REFACTOR: Signature updated to support dual-save strategy. ---
     * Saves the settings of a FilterManager to a JSON file. The behavior now
     * depends on the timestamp provided.
     * @param filter The FilterManager instance to save.
     * @param filterName The base name for the config file (e.g., "ph_filter").
     * @param sessionTimestamp If "default", saves to the primary operational file (e.g., "ph_filter.json").
     * If a valid timestamp string is provided, saves a unique, versioned log file.
     * @param is_saved_state (DEPRECATED) This parameter is no longer used and will be removed in a future refactor.
     * @return True if saving was successful, false otherwise.
     */
    bool saveFilterSettings(FilterManager& filter, const char* filterName, const char* sessionTimestamp, bool is_saved_state = false);

    bool loadFilterSettings(FilterManager& filter, const char* filterName, bool is_saved_state = false);

private:
    FaultHandler* _faultHandler;
    SdManager* _sdManager;
    bool _initialized;
};

#endif // CONFIG_MANAGER_H