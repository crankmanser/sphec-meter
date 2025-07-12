#pragma once

#include "hal/INA219_Driver.h"
#include "managers/storage/StorageManager.h"
#include <vector>
#include <utility>

struct PowerManagerState {
    float stateOfHealthPercent = 100.0f;
    double chargeCycles = 0.0;
    double accumulatedDischargeWh = 0.0;
    float remainingEnergyWh = 21.6f;
};

class PowerManager {
public:
    PowerManager(INA219_Driver& ina219, StorageManager& storage);
    void begin();
    void update();

    // Public Getters
    float getVoltage();
    float getCurrent();
    float getPower();
    float getStateOfCharge();
    float getStateOfHealth();
    double getChargeCycles();
    bool isCharging();

private:
    INA219_Driver& _ina219;
    StorageManager& _storage;
    PowerManagerState _state;

    // Live state
    float _voltage_V;
    float _current_mA;
    bool _is_charging;
    unsigned long _last_update_ms;

    // Lookup Tables & Constants
    const std::vector<std::pair<float, float>> OCV_SOC_TABLE;
    const std::vector<std::pair<int, float>> SOH_CYCLE_TABLE;
    static constexpr float NOMINAL_WATT_HOURS = 21.6f;
    static constexpr float CHARGE_CURRENT_CUTOFF_mA = 100.0f;

    // Private Methods
    void loadState();
    void saveState();
    void reconcileStateFromOCV(); // <<< NEW
    float getSocFromVoltage(float voltage);
    float getModelSoh();
};