// File Path: /lib/PowerMonitor/src/PowerMonitor.cpp

#include "PowerMonitor.h"
#include <Arduino.h>

constexpr float PowerMonitor::NOMINAL_WATT_HOURS;

PowerMonitor::PowerMonitor() :
    _faultHandler(nullptr),
    _ina219(nullptr),
    _storage(nullptr),
    _initialized(false),
    _voltage_V(0.0f),
    _current_mA(0.0f),
    _is_charging(false),
    _last_update_ms(0),
    _currentBufferIndex(0)
{
    for(int i = 0; i < CURRENT_FILTER_WINDOW_SIZE; ++i) {
        _currentBuffer[i] = 0.0f;
    }
}

bool PowerMonitor::begin(FaultHandler& faultHandler, INA219_Driver& ina219, I_StorageProvider& storage) {
    _faultHandler = &faultHandler;
    _ina219 = &ina219;
    _storage = &storage;
    loadState();
    reconcileStateFromOCV();
    _last_update_ms = millis();
    _initialized = true;
    return true;
}

void PowerMonitor::update() {
    if (!_initialized) return;

    unsigned long now = millis();
    if (now - _last_update_ms < 1000) return;

    unsigned long delta_ms = now - _last_update_ms;
    _last_update_ms = now;

    _voltage_V = _ina219->getBusVoltage();
    
    // --- FIX: Implement moving average filter for current ---
    _currentBuffer[_currentBufferIndex] = _ina219->getCurrent_mA();
    _currentBufferIndex = (_currentBufferIndex + 1) % CURRENT_FILTER_WINDOW_SIZE;
    
    float sum = 0.0f;
    for(int i = 0; i < CURRENT_FILTER_WINDOW_SIZE; ++i) {
        sum += _currentBuffer[i];
    }
    _current_mA = sum / CURRENT_FILTER_WINDOW_SIZE;
    
    bool was_charging = _is_charging;
    _is_charging = (_current_mA > CHARGE_CURRENT_CUTOFF_mA);

    if (!_is_charging && was_charging) {
        delay(5000);
        reconcileStateFromOCV();
    }

    float power_W = _voltage_V * (_current_mA / 1000.0f);
    float energy_delta_Wh = power_W * (delta_ms / 3600000.0f);

    _state.remainingEnergyWh += energy_delta_Wh;

    if (power_W < 0) {
        _state.accumulatedDischargeWh -= energy_delta_Wh;
    }

    _state.chargeCycles = _state.accumulatedDischargeWh / NOMINAL_WATT_HOURS;
    _state.stateOfHealthPercent = getSohFromCycles();

    float max_capacity_wh = NOMINAL_WATT_HOURS * (_state.stateOfHealthPercent / 100.0f);
    _state.remainingEnergyWh = constrain(_state.remainingEnergyWh, 0.0f, max_capacity_wh);
}

float PowerMonitor::getSOC() {
    float max_capacity_wh = NOMINAL_WATT_HOURS * (_state.stateOfHealthPercent / 100.0f);
    if (max_capacity_wh <= 0.0f) return 0.0f;
    return constrain((_state.remainingEnergyWh / max_capacity_wh) * 100.0f, 0.0f, 100.0f);
}

// ... (Rest of PowerMonitor.cpp is unchanged) ...
float PowerMonitor::getSOH() { return _state.stateOfHealthPercent; }
bool PowerMonitor::isCharging() { return _is_charging; }
float PowerMonitor::getVoltage() { return _voltage_V; }
float PowerMonitor::getCurrent() { return _current_mA; }
void PowerMonitor::reconcileStateFromOCV() {
    float ocv_voltage = _ina219->getBusVoltage();
    for (int i = 0; i < 4; i++) {
        delay(50);
        ocv_voltage = (ocv_voltage + _ina219->getBusVoltage()) / 2.0f;
    }
    float true_soc = getSocFromVoltage(ocv_voltage);
    float max_capacity_wh = NOMINAL_WATT_HOURS * (getSohFromCycles() / 100.0f);
    _state.remainingEnergyWh = max_capacity_wh * (true_soc / 100.0f);
    saveState();
}
void PowerMonitor::loadState() {
    StaticJsonDocument<256> doc;
    if (_storage->loadJson("/power_state.json", doc)) {
        _state.stateOfHealthPercent = doc["soh"] | 100.0f;
        _state.chargeCycles = doc["cycles"] | 0.0;
        _state.accumulatedDischargeWh = doc["accumulatedDischarge"] | 0.0;
        _state.remainingEnergyWh = doc["remainingEnergy"] | NOMINAL_WATT_HOURS;
    } else {
        saveState();
    }
}
void PowerMonitor::saveState() {
    StaticJsonDocument<256> doc;
    doc["soh"] = _state.stateOfHealthPercent;
    doc["cycles"] = _state.chargeCycles;
    doc["accumulatedDischarge"] = _state.accumulatedDischargeWh;
    doc["remainingEnergy"] = _state.remainingEnergyWh;
    _storage->saveJson("/power_state.json", doc);
}
float PowerMonitor::getSocFromVoltage(float voltage) {
    if (voltage >= OCV_SOC_TABLE.back().first) return OCV_SOC_TABLE.back().second;
    if (voltage <= OCV_SOC_TABLE.front().first) return OCV_SOC_TABLE.front().second;
    for (size_t i = 0; i < OCV_SOC_TABLE.size() - 1; ++i) {
        if (voltage >= OCV_SOC_TABLE[i].first && voltage < OCV_SOC_TABLE[i+1].first) {
            float v_range = OCV_SOC_TABLE[i+1].first - OCV_SOC_TABLE[i].first;
            if (v_range <= 0) continue;
            float soc_range = OCV_SOC_TABLE[i+1].second - OCV_SOC_TABLE[i].second;
            float v_pos = voltage - OCV_SOC_TABLE[i].first;
            return OCV_SOC_TABLE[i].second + (soc_range * (v_pos / v_range));
        }
    }
    return 0.0f;
}
float PowerMonitor::getSohFromCycles() {
    float cycles = _state.chargeCycles;
    if (cycles <= SOH_CYCLE_TABLE.front().first) return SOH_CYCLE_TABLE.front().second;
    if (cycles >= SOH_CYCLE_TABLE.back().first) return SOH_CYCLE_TABLE.back().second;
    for (size_t i = 0; i < SOH_CYCLE_TABLE.size() - 1; ++i) {
        if (cycles >= SOH_CYCLE_TABLE[i].first && cycles < SOH_CYCLE_TABLE[i+1].first) {
            float cycle_range = SOH_CYCLE_TABLE[i+1].first - SOH_CYCLE_TABLE[i].first;
            if (cycle_range <= 0) continue;
            float soh_range = SOH_CYCLE_TABLE[i+1].second - SOH_CYCLE_TABLE[i].second;
            float cycle_pos = cycles - SOH_CYCLE_TABLE[i].first;
            return SOH_CYCLE_TABLE[i].second + (soh_range * (cycle_pos / cycle_range));
        }
    }
    return 100.0f;
}