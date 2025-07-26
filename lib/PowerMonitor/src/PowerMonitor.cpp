// File Path: /lib/PowerMonitor/src/PowerMonitor.cpp

#include "PowerMonitor.h"
#include <Arduino.h> // For millis() and constrain()

// --- FIX: Define the static constant for the linker ---
constexpr float PowerMonitor::NOMINAL_WATT_HOURS;

/**
 * @brief Constructor for the PowerMonitor.
 *
 * Initializes all member variables to a safe default state.
 * Dependencies are injected via the begin() method.
 */
PowerMonitor::PowerMonitor() :
    _faultHandler(nullptr),
    _ina219(nullptr),
    _storage(nullptr),
    _initialized(false),
    _voltage_V(0.0f),
    _current_mA(0.0f),
    _is_charging(false),
    _last_update_ms(0)
{}

/**
 * @brief Initializes the PowerMonitor.
 *
 * Stores references to its dependencies, loads its persistent state from storage,
 * and performs an initial OCV reconciliation to ensure the battery gauge is
 * accurate from the start.
 *
 * @param faultHandler A reference to the global fault handler.
 * @param ina219 A reference to the INA219 HAL driver.
 * @param storage A reference to a storage provider for persistence.
 * @return True if initialization is successful, false otherwise.
 */
bool PowerMonitor::begin(FaultHandler& faultHandler, INA219_Driver& ina219, I_StorageProvider& storage) {
    _faultHandler = &faultHandler;
    _ina219 = &ina219;
    _storage = &storage;

    loadState(); // Load the last known state from storage.
    reconcileStateFromOCV(); // Perform initial calibration based on resting voltage.

    _last_update_ms = millis();
    _initialized = true;
    return true;
}

/**
 * @brief Main update loop for the PowerMonitor.
 *
 * This is the core of the Coulomb Counting logic. It measures the time elapsed
 * since the last update, calculates the energy (in Watt-hours) that has flowed
 * into or out of the battery during that time, and updates the state accordingly.
 */
void PowerMonitor::update() {
    if (!_initialized) return;

    unsigned long now = millis();
    // To prevent rapid, unnecessary updates, we only run the logic once per second.
    if (now - _last_update_ms < 1000) return;

    unsigned long delta_ms = now - _last_update_ms;
    _last_update_ms = now;

    _voltage_V = _ina219->getBusVoltage();
    _current_mA = _ina219->getCurrent_mA();

    bool was_charging = _is_charging;
    // Charging is defined as a positive current greater than our defined cutoff threshold.
    _is_charging = (_current_mA > CHARGE_CURRENT_CUTOFF_mA);

    // If we just stopped charging, the battery voltage will settle. This is an
    // ideal time to perform OCV reconciliation to correct any accumulated error.
    if (!_is_charging && was_charging) {
        delay(5000); // Wait 5 seconds for voltage to stabilize.
        reconcileStateFromOCV();
    }

    // --- Coulomb Counting Math ---
    // Power (W) = Voltage (V) * Current (A)
    float power_W = _voltage_V * (_current_mA / 1000.0f);
    // Energy (Wh) = Power (W) * time (h)
    float energy_delta_Wh = power_W * (delta_ms / 3600000.0f);

    // Update remaining energy. If discharging, energy_delta_Wh will be negative.
    _state.remainingEnergyWh += energy_delta_Wh;

    // Only accumulate discharge when power is negative.
    if (power_W < 0) {
        // We subtract the negative energy delta to add a positive value to the accumulator.
        _state.accumulatedDischargeWh -= energy_delta_Wh;
    }

    // Update KPIs
    _state.chargeCycles = _state.accumulatedDischargeWh / NOMINAL_WATT_HOURS;
    _state.stateOfHealthPercent = getSohFromCycles();

    // Clamp the remaining energy to prevent it from exceeding the battery's
    // current maximum capacity or dropping below zero.
    float max_capacity_wh = NOMINAL_WATT_HOURS * (_state.stateOfHealthPercent / 100.0f);
    _state.remainingEnergyWh = constrain(_state.remainingEnergyWh, 0.0f, max_capacity_wh);
}

// --- Public API Getters ---

float PowerMonitor::getSOC() {
    float max_capacity_wh = NOMINAL_WATT_HOURS * (_state.stateOfHealthPercent / 100.0f);
    if (max_capacity_wh <= 0.0f) return 0.0f;
    float soc = (_state.remainingEnergyWh / max_capacity_wh) * 100.0f;
    return constrain(soc, 0.0f, 100.0f);
}

float PowerMonitor::getSOH() { return _state.stateOfHealthPercent; }
bool PowerMonitor::isCharging() { return _is_charging; }
float PowerMonitor::getVoltage() { return _voltage_V; }
float PowerMonitor::getCurrent() { return _current_mA; }


// --- Private Helper Methods ---

/**
 * @brief Performs Open-Circuit Voltage (OCV) reconciliation.
 *
 * This method reads the resting voltage of the battery, uses the OCV_SOC_TABLE
 * to determine the "true" State of Charge, and recalibrates the internal
 * remainingEnergyWh value. This corrects for drift that accumulates over time.
 */
void PowerMonitor::reconcileStateFromOCV() {
    // Take several readings to get a stable average resting voltage.
    float ocv_voltage = _ina219->getBusVoltage();
    for (int i = 0; i < 4; i++) {
        delay(50);
        ocv_voltage = (ocv_voltage + _ina219->getBusVoltage()) / 2.0f;
    }

    float true_soc = getSocFromVoltage(ocv_voltage);
    float max_capacity_wh = NOMINAL_WATT_HOURS * (getSohFromCycles() / 100.0f);
    _state.remainingEnergyWh = max_capacity_wh * (true_soc / 100.0f);
    
    saveState(); // Persist the corrected state immediately.
}

/**
 * @brief Loads the PowerMonitor's state from the storage provider.
 */
void PowerMonitor::loadState() {
    StaticJsonDocument<256> doc;
    if (_storage->loadJson("/power_state.json", doc)) {
        _state.stateOfHealthPercent = doc["soh"] | 100.0f;
        _state.chargeCycles = doc["cycles"] | 0.0;
        _state.accumulatedDischargeWh = doc["accumulatedDischarge"] | 0.0;
        _state.remainingEnergyWh = doc["remainingEnergy"] | NOMINAL_WATT_HOURS;
    } else {
        // If loading fails, we save the default state to create the file.
        saveState();
    }
}

/**
 * @brief Saves the PowerMonitor's current state to the storage provider.
 */
void PowerMonitor::saveState() {
    StaticJsonDocument<256> doc;
    doc["soh"] = _state.stateOfHealthPercent;
    doc["cycles"] = _state.chargeCycles;
    doc["accumulatedDischarge"] = _state.accumulatedDischargeWh;
    doc["remainingEnergy"] = _state.remainingEnergyWh;
    _storage->saveJson("/power_state.json", doc);
}

/**
 * @brief Gets State of Charge from voltage using the lookup table.
 *
 * Performs linear interpolation between the points in the OCV_SOC_TABLE
 * to get an accurate SOC value for a given resting voltage.
 */
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
    return 0.0f; // Should not be reached
}

/**
 * @brief Gets State of Health from charge cycles using the lookup table.
 *
 * Performs linear interpolation between the points in the SOH_CYCLE_TABLE
 * to estimate the battery's current health based on its usage history.
 */
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
    return 100.0f; // Should not be reached
}