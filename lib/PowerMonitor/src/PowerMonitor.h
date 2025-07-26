// File Path: /lib/PowerMonitor/src/PowerMonitor.h

#ifndef POWER_MONITOR_H
#define POWER_MONITOR_H

#include <INA219_Driver.h>
#include <I_StorageProvider.h> // Using interface for storage
#include <FaultHandler.h>
#include <vector>
#include <utility> // For std::pair

/**
 * @struct PowerMonitorState
 * @brief Holds all the persistent data for the PowerMonitor.
 *
 * This struct is designed to be serialized to a JSON file, allowing the
 * battery gauge's state to be preserved across device reboots.
 */
struct PowerMonitorState {
    float stateOfHealthPercent = 100.0f;     // SOH, starts at 100%
    double chargeCycles = 0.0;               // Equivalent full charge cycles
    double accumulatedDischargeWh = 0.0;     // Total energy discharged
    float remainingEnergyWh = 21.6f;         // Remaining energy in Watt-hours
};

/**
 * @class PowerMonitor
 * @brief An intelligent cabinet for managing the device's battery state.
 *
 * This cabinet implements a hybrid model for battery gauging, combining
 * real-time Coulomb Counting with periodic Open-Circuit Voltage (OCV)
 * reconciliation to provide an accurate State of Charge (SOC) and
 * State of Health (SOH).
 */
class PowerMonitor {
public:
    /**
     * @brief Constructor for the PowerMonitor.
     */
    PowerMonitor();

    /**
     * @brief Initializes the PowerMonitor.
     * @param faultHandler A reference to the global fault handler.
     * @param ina219 A reference to the INA219 HAL driver.
     * @param storage A reference to a storage provider for persistence.
     * @return True if initialization is successful, false otherwise.
     */
    bool begin(FaultHandler& faultHandler, INA219_Driver& ina219, I_StorageProvider& storage);

    /**
     * @brief Main update loop for the PowerMonitor. Should be called periodically.
     *
     * This method performs the real-time Coulomb Counting by reading the current
     * from the INA219 and integrating it over time to track energy flow.
     */
    void update();

    // --- Public API ---
    float getSOC();
    float getSOH();
    bool isCharging();
    float getVoltage();
    float getCurrent();

private:
    // --- Injected Dependencies ---
    FaultHandler* _faultHandler;
    INA219_Driver* _ina219;
    I_StorageProvider* _storage;
    PowerMonitorState _state;
    bool _initialized;

    // --- Live State (not persisted) ---
    float _voltage_V;
    float _current_mA;
    bool _is_charging;
    unsigned long _last_update_ms;

    // --- Constants & Lookup Tables (derived from INR18650-30Q datasheet for 2S pack) ---
    static constexpr float NOMINAL_WATT_HOURS = 21.6f; // (3.6V * 3.0Ah) * 2 cells
    static constexpr float CHARGE_CURRENT_CUTOFF_mA = 100.0f;

    // OCV vs SOC lookup table for a 2S pack (voltage x2 from datasheet)
    const std::vector<std::pair<float, float>> OCV_SOC_TABLE = {
        {6.00, 0.0}, {6.70, 10.0}, {7.00, 20.0}, {7.16, 30.0},
        {7.28, 40.0}, {7.40, 50.0}, {7.52, 60.0}, {7.66, 70.0},
        {7.84, 80.0}, {8.08, 90.0}, {8.40, 100.0}
    };

    // Charge Cycles vs SOH lookup table
    const std::vector<std::pair<int, float>> SOH_CYCLE_TABLE = {
        {0, 100.0}, {50, 96.0}, {100, 92.0}, {150, 88.0},
        {200, 83.0}, {250, 78.0}, {300, 73.8}
    };

    // --- Private Methods ---
    void loadState();
    void saveState();
    void reconcileStateFromOCV();
    float getSocFromVoltage(float voltage);
    float getSohFromCycles();
};

#endif // POWER_MONITOR_H