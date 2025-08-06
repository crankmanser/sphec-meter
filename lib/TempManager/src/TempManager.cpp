// File Path: /lib/TempManager/src/TempManager.cpp
// MODIFIED FILE

#include "TempManager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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

bool TempManager::begin(FaultHandler& faultHandler) {
    _faultHandler = &faultHandler;

    _dallasSensors.begin();
    _dht.begin();

    if (_dallasSensors.getDeviceCount() == 0) {
        return false;
    }
    
    _dallasSensors.setResolution(12);
    // --- DEFINITIVE FIX: Configure the library for non-blocking reads ---
    _dallasSensors.setWaitForConversion(false);
    
    _initialized = true;
    return true;
}

/**
 * @brief --- MODIFIED: This function now only READS sensor values. ---
 * The long-running "request" operation is moved to the main task loop
 * to make the process RTOS-friendly and prevent watchdog timeouts.
 */
void TempManager::update() {
    if (!_initialized) return;

    // --- Read DS18B20 Probe Sensor ---
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

/**
 * @brief --- NEW: Starts the temperature conversion on the DS18B20. ---
 * This is a non-blocking call.
 */
void TempManager::requestProbeTemperature() {
    if (!_initialized) return;
    _dallasSensors.requestTemperatures();
}


float TempManager::getProbeTemp() {
    return _probeTempC;
}

float TempManager::getAmbientTemp() {
    return _ambientTempC;
}

float TempManager::getHumidity() {
    return _humidity;
}

void TempManager::setProbeTempOffset(float offset) {
    _probeTempOffset = offset;
}

void TempManager::setAmbientTempOffset(float offset) {
    _ambientTempOffset = offset;
}