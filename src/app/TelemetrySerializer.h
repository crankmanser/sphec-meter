#pragma once

#include "data_models/SensorData_types.h"
#include "managers/power/PowerManager.h"
#include <ArduinoJson.h>
#include <string>

/**
 * @class TelemetrySerializer
 * @brief A cabinet responsible for creating a single, reusable JSON telemetry string.
 *
 * This cabinet follows the Single Responsibility Principle. Its only job is to
 * read from the various data sources (g_processed_data, PowerManager) and
 * serialize them into a JSON string once per application cycle. This decouples
 * JSON serialization from the connectivity managers (BLE, Wi-Fi, MQTT),
 * resolving the critical memory allocation issue identified in the project manifest.
 */
class TelemetrySerializer {
public:
    /**
     * @brief Constructs the TelemetrySerializer.
     * @param processed_data A pointer to the global struct holding final sensor values.
     * @param power_manager A reference to the PowerManager to get battery status.
     */
    TelemetrySerializer(const ProcessedSensorData* processed_data, PowerManager& power_manager);

    /**
     * @brief Initializes the serializer.
     */
    void begin();

    /**
     * @brief Updates the internal telemetry string by re-reading and serializing data.
     */
    void update();

    /**
     * @brief Gets a constant reference to the last generated telemetry JSON string.
     * @return A const reference to the telemetry string.
     */
    const std::string& getSerializedTelemetry() const;

private:
    const ProcessedSensorData* _processed_data;
    PowerManager& _power_manager;
    std::string _telemetry_json;
    StaticJsonDocument<512> _json_doc;
};