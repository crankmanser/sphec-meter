// File Path: /lib/PowerMonitor/src/PowerMonitor.h

#ifndef POWER_MONITOR_H
#define POWER_MONITOR_H

#include <INA219_Driver.h>
#include <I_StorageProvider.h>
#include <FaultHandler.h>
#include <vector>
#include <utility>
#include <numeric>

#define CURRENT_FILTER_WINDOW_SIZE 10

struct PowerMonitorState {
    float stateOfHealthPercent = 100.0f;
    double chargeCycles = 0.0;
    double accumulatedDischargeWh = 0.0;
    float remainingEnergyWh = 21.6f;
};

class PowerMonitor {
public:
    PowerMonitor();
    bool begin(FaultHandler& faultHandler, INA219_Driver& ina219, I_StorageProvider& storage);
    void update();
    float getSOC();
    float getSOH();
    bool isCharging();
    float getVoltage();
    float getCurrent();

private:
    FaultHandler* _faultHandler;
    INA219_Driver* _ina219;
    I_StorageProvider* _storage;
    PowerMonitorState _state;
    bool _initialized;
    float _voltage_V;
    float _current_mA;
    bool _is_charging;
    unsigned long _last_update_ms;

    // --- NEW: Buffer for moving average filter on current ---
    float _currentBuffer[CURRENT_FILTER_WINDOW_SIZE];
    int _currentBufferIndex;

    static constexpr float NOMINAL_WATT_HOURS = 21.6f;
    static constexpr float CHARGE_CURRENT_CUTOFF_mA = 100.0f;
    const std::vector<std::pair<float, float>> OCV_SOC_TABLE = {
        {6.00, 0.0}, {6.70, 10.0}, {7.00, 20.0}, {7.16, 30.0},
        {7.28, 40.0}, {7.40, 50.0}, {7.52, 60.0}, {7.66, 70.0},
        {7.84, 80.0}, {8.08, 90.0}, {8.40, 100.0}
    };
    const std::vector<std::pair<int, float>> SOH_CYCLE_TABLE = {
        {0, 100.0}, {50, 96.0}, {100, 92.0}, {150, 88.0},
        {200, 83.0}, {250, 78.0}, {300, 73.8}
    };

    void loadState();
    void saveState();
    void reconcileStateFromOCV();
    float getSocFromVoltage(float voltage);
    float getSohFromCycles();
};

#endif // POWER_MONITOR_H