// File Path: /lib/TempManager/src/TempManager.cpp

#include "TempManager.h"

// --- Constructor ---
TempManager::TempManager() :
    _faultHandler(nullptr),
    _initialized(false),
    _oneWire(ONE_WIRE_BUS_PIN),
    _dallasSensors(&_oneWire),
    _dht(DHT_PIN, DHT_TYPE),
    _probeTempC(NAN),
    _ambientTempC(NAN),
    _humidity(NAN),
    _probeTempOffset(0.0f),
    _ambientTempOffset(0.0f)
{}

// --- Initialization ---
bool TempManager::begin(FaultHandler& faultHandler) {
    _faultHandler = &faultHandler;

    _dallasSensors.begin();
    _dht.begin();

    // Check if at least one DS18B20 is found
    if (_dallasSensors.getDeviceCount() == 0) {
        // This could be a non-fatal warning if the sensor is optional
        // _faultHandler->trigger_fault("DS18B20_NOT_FOUND", "No DS18B20 sensors found", __FILE__, __LINE__);
        return false;
    }
    
    _dallasSensors.setResolution(12); // Set to highest resolution
    _initialized = true;
    return true;
}

/**
 * @brief Reads temperature and humidity from all sensors.
 * It reads the raw value and applies the stored calibration offset.
 */
void TempManager::update() {
    if (!_initialized) return;

    // --- Read DS18B20 Probe Sensor ---
    _dallasSensors.requestTemperatures(); 
    float rawProbeTemp = _dallasSensors.getTempCByIndex(0);
    if (rawProbeTemp != DEVICE_DISCONNECTED_C) {
        _probeTempC = rawProbeTemp + _probeTempOffset;
    }

    // --- Read DHT11 Ambient Sensor ---
    float rawAmbientTemp = _dht.readTemperature();
    float humidity = _dht.readHumidity();

    if (!isnan(rawAmbientTemp)) {
        _ambientTempC = rawAmbientTemp + _ambientTempOffset;
    }
    if (!isnan(humidity)) {
        _humidity = humidity;
    }
}

// --- Public Getters ---
float TempManager::getProbeTemp() {
    return _probeTempC;
}

float TempManager::getAmbientTemp() {
    return _ambientTempC;
}

float TempManager::getHumidity() {
    return _humidity;
}

// --- Calibration Methods ---
void TempManager::setProbeTempOffset(float offset) {
    _probeTempOffset = offset;
}

void TempManager::setAmbientTempOffset(float offset) {
    _ambientTempOffset = offset;
}