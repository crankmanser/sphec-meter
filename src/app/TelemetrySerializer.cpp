#include "app/TelemetrySerializer.h"
#include "DebugMacros.h"

TelemetrySerializer::TelemetrySerializer(const ProcessedSensorData* processed_data, PowerManager& power_manager) :
    _processed_data(processed_data),
    _power_manager(power_manager)
{}

void TelemetrySerializer::begin() {
    LOG_MANAGER("TelemetrySerializer initialized.\n");
    // Pre-populate the string to avoid issues on the first loop
    update();
}

void TelemetrySerializer::update() {
    // Populate the JSON document with the latest data
    _json_doc["liquid_temp"] = _processed_data->liquid_temp_c;
    _json_doc["ambient_temp"] = _processed_data->ambient_temp_c;
    _json_doc["humidity"] = _processed_data->ambient_humidity_percent;
    _json_doc["ph"] = _processed_data->ph_value;
    _json_doc["ec"] = _processed_data->ec_value;
    _json_doc["light_lvl"] = _processed_data->light_level_percent;
    _json_doc["soc"] = _power_manager.getStateOfCharge();

    // Serialize the document into our internal string variable
    _telemetry_json.clear();
    serializeJson(_json_doc, _telemetry_json);
}

const std::string& TelemetrySerializer::getSerializedTelemetry() const {
    return _telemetry_json;
}