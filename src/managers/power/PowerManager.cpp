// src/managers/power/PowerManager.cpp
#include "PowerManager.h"
#include "DebugMacros.h"

// Constructor is unchanged...
PowerManager::PowerManager(INA219_Driver& ina219, StorageManager& storage) :
    _ina219(ina219),
    _storage(storage),
    _voltage_V(0.0f),
    _current_mA(0.0f),
    _is_charging(false),
    _last_update_ms(0),
    OCV_SOC_TABLE({
        {5.00, 0.0}, {6.10, 5.0}, {6.36, 10.0}, {6.54, 15.0}, {6.68, 20.0},
        {6.80, 25.0}, {6.92, 30.0}, {7.02, 35.0}, {7.12, 40.0}, {7.22, 45.0},
        {7.30, 50.0}, {7.38, 55.0}, {7.46, 60.0}, {7.54, 65.0}, {7.62, 70.0},
        {7.70, 75.0}, {7.78, 80.0}, {7.88, 85.0}, {8.00, 90.0}, {8.16, 95.0},
        {8.40, 100.0}
    }),
    SOH_CYCLE_TABLE({
        {0, 100.0}, {50, 96.0}, {100, 92.0}, {150, 88.0},
        {200, 83.0}, {250, 78.0}, {300, 73.8}
    })
{}

void PowerManager::begin() {
    loadState();
    reconcileStateFromOCV();
    _last_update_ms = millis();
}

void PowerManager::update() {
    unsigned long now = millis();
    // This check is to ensure the update only runs once per second
    if (now - _last_update_ms < 1000) return; 

    unsigned long delta_ms = now - _last_update_ms;
    _last_update_ms = now;

    _voltage_V = _ina219.getBusVoltage();
    _current_mA = _ina219.getCurrent_mA();

    bool was_charging = _is_charging;
    // --- FIX 1: Correct charging detection logic ---
    // If current is positive and greater than the cutoff, we are charging.
    _is_charging = (_current_mA > CHARGE_CURRENT_CUTOFF_mA);

    if (!_is_charging && was_charging) {
        LOG_MANAGER("Charger unplugged. Recalibrating SOC from resting voltage.\n");
        delay(5000); 
        reconcileStateFromOCV();
    }

    float power_W = _voltage_V * (_current_mA / 1000.0f);
    float energy_delta_Wh = power_W * (delta_ms / 3600000.0f);

    // --- FIX 2: Correct Coulomb Counting direction ---
    // Since charging current is positive, we ADD the energy delta.
    // Since discharging current is negative, this will correctly subtract energy.
    _state.remainingEnergyWh += energy_delta_Wh;

    // --- FIX 3 & 4: Correct accumulated discharge calculation ---
    // We only accumulate when power is negative (discharging).
    if (power_W < 0) {
        // We subtract the negative energy delta to add a positive value to the accumulator.
        _state.accumulatedDischargeWh -= energy_delta_Wh;
    }

    _state.chargeCycles = _state.accumulatedDischargeWh / NOMINAL_WATT_HOURS;
    _state.stateOfHealthPercent = getModelSoh();
    float max_capacity_wh = NOMINAL_WATT_HOURS * (_state.stateOfHealthPercent / 100.0f);
    _state.remainingEnergyWh = constrain(_state.remainingEnergyWh, 0.0f, max_capacity_wh);
}

void PowerManager::reconcileStateFromOCV() {
    LOG_MANAGER("Performing OCV reconciliation...\n");
    float ocv_voltage = _ina219.getBusVoltage();
    for (int i=0; i<4; i++) {
        delay(50);
        ocv_voltage = (ocv_voltage + _ina219.getBusVoltage()) / 2.0;
    }

    float true_soc = getSocFromVoltage(ocv_voltage);
    float max_capacity_wh = NOMINAL_WATT_HOURS * (getModelSoh() / 100.0f);
    _state.remainingEnergyWh = max_capacity_wh * (true_soc / 100.0f);

    LOG_MANAGER("Reconciliation complete. OCV=%.2fV -> True SOC=%.1f%% -> New Rem_Wh: %.2f\n",
        ocv_voltage, true_soc, _state.remainingEnergyWh);
    
    saveState();
}

float PowerManager::getVoltage() { return _voltage_V; }
float PowerManager::getCurrent() { return _current_mA; }
float PowerManager::getPower() { return _voltage_V * (_current_mA / 1000.0f); }
double PowerManager::getChargeCycles() { return _state.chargeCycles; }
bool PowerManager::isCharging() { return _is_charging; }

float PowerManager::getStateOfCharge() {
    float max_capacity_wh = NOMINAL_WATT_HOURS * (_state.stateOfHealthPercent / 100.0f);
    if (max_capacity_wh <= 0.0) return 0.0;
    float soc = (_state.remainingEnergyWh / max_capacity_wh) * 100.0f;
    return constrain(soc, 0.0, 100.0);
}

float PowerManager::getStateOfHealth() {
    return _state.stateOfHealthPercent;
}

void PowerManager::loadState() {
    if (!_storage.loadState(ConfigType::POWER_MANAGER_STATE, (uint8_t*)&_state, sizeof(_state))) {
        LOG_MANAGER("No saved power state found. Initializing with defaults.\n");
        saveState();
    } else {
        LOG_MANAGER("Power state loaded successfully.\n");
    }
}

void PowerManager::saveState() {
    if (!_storage.saveState(ConfigType::POWER_MANAGER_STATE, (const uint8_t*)&_state, sizeof(_state))) {
        LOG_MAIN("[PM_ERROR] Failed to save power state!\n");
    }
}

float PowerManager::getModelSoh() {
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
    return 100.0;
}

float PowerManager::getSocFromVoltage(float voltage) {
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
    return 0.0;
}