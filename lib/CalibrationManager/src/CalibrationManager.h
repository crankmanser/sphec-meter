// File Path: /lib/CalibrationManager/src/CalibrationManager.h
// MODIFIED FILE

#ifndef CALIBRATION_MANAGER_H
#define CALIBRATION_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FaultHandler.h>

#define CALIBRATION_POINT_COUNT 3

struct CalibrationPoint {
    double voltage;
    double value;
};

struct CalibrationModel {
    double coeff_a = 0.0;
    double coeff_b = 0.0;
    double coeff_c = 0.0;
    double calibrationTemperature = 25.0;
    CalibrationPoint points[CALIBRATION_POINT_COUNT];

    double qualityScore = 0.0;
    double sensorDrift = 0.0;
    double healthScore = 0.0;
    
    double neutralVoltage = 0.0;
    double zeroPointDrift = 0.0;

    bool isCalibrated = false;
    time_t lastCalibratedTimestamp = 0;
};

class CalibrationManager {
public:
    CalibrationManager();
    bool begin(FaultHandler& faultHandler);
    double getCalibratedValue(double filteredVoltage);
    double getCompensatedValue(double rawValue, double measuredTemperature, bool isEC = false);
    void startNewCalibration();
    bool addCalibrationPoint(double voltage, double knownValue, double temperature);
    double calculateNewModel(const CalibrationModel& previousModel);
    const CalibrationModel& getCurrentModel() const;
    CalibrationModel& getMutableCurrentModel();
    const CalibrationModel& getNewModel() const;
    void acceptNewModel();
    // --- DEFINITIVE FIX: Change signature to accept JsonObject ---
    void serializeModel(const CalibrationModel& model, JsonObject& doc);
    bool deserializeModel(CalibrationModel& model, JsonDocument& doc);

private:
    FaultHandler* _faultHandler;
    bool _initialized;
    CalibrationModel _currentModel;
    CalibrationModel _newModel;
    int _newPointsCount;
};

#endif // CALIBRATION_MANAGER_H