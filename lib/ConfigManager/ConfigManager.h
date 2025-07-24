// File Path: /lib/ConfigManager/ConfigManager.h

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <ArduinoJson.h>
#include <FaultHandler.h> // Depends on FaultHandler for error reporting.

// Forward declaration for StorageEngine to avoid circular dependencies.
// The ConfigManager will use the StorageEngine but doesn't need its full definition here.
class StorageEngine; 

/**
 * @class ConfigManager
 * @brief A unified cabinet for managing all device configurations.
 * * As per the architectural blueprint, this class provides a single interface
 * for all configuration data. It will internally decide whether to fetch data
 * from the fast internal flash (NVS) or from the SD card via the StorageEngine.
 *
 */
class ConfigManager {
public:
    /**
     * @brief Constructor for the ConfigManager.
     */
    ConfigManager();

    /**
     * @brief Initializes the ConfigManager.
     * @param faultHandler A reference to the global fault handler for error reporting.
     * @return True if initialization is successful, false otherwise.
     */
    bool begin(FaultHandler& faultHandler);

private:
    FaultHandler* _faultHandler; // Pointer to the system fault handler.
    bool _initialized;
};

#endif // CONFIG_MANAGER_H