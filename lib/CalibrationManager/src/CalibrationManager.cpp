// File Path: /lib/CalibrationManager/src/CalibrationManager.cpp
// MODIFIED FILE

#include "CalibrationManager.h"
#include <cmath>
#include <numeric>

double getTemperatureCorrectedBufferValue(double nominalValue, double temperature);

CalibrationManager::CalibrationManager() : _faultHandler(nullptr), _initialized(false), _newPointsCount(0) {}
bool CalibrationManager::begin(FaultHandler& faultHandler) { _faultHandler = &faultHandler; _initialized = true; return true; }
double CalibrationManager::getCalibratedValue(double filteredVoltage) {
    if (!_currentModel.isCalibrated) return 0.0;
    return (_currentModel.coeff_a * std::pow(filteredVoltage, 2)) + (_currentModel.coeff_b * filteredVoltage) + _currentModel.coeff_c;
}
double CalibrationManager::getCompensatedValue(double rawValue, double measuredTemperature, bool isEC) {
    if (isEC) {
        const double ecAlpha = 0.0191; const double refTemp = 25.0;
        return rawValue / (1.0 + ecAlpha * (measuredTemperature - refTemp));
    } else {
        const double phAlpha = 0.003; const double neutralPh = 7.0;
        return rawValue + ((rawValue - neutralPh) * (measuredTemperature - _currentModel.calibrationTemperature) * phAlpha);
    }
}
void CalibrationManager::startNewCalibration() { _newPointsCount = 0; _newModel = CalibrationModel(); }
bool CalibrationManager::addCalibrationPoint(double voltage, double knownValue, double temperature) {
    if (_newPointsCount >= CALIBRATION_POINT_COUNT) return false;
    double correctedValue = getTemperatureCorrectedBufferValue(knownValue, temperature);
    _newModel.points[_newPointsCount].voltage = voltage;
    _newModel.points[_newPointsCount].value = correctedValue;
    _newPointsCount++;
    if (_newPointsCount == 1) { _newModel.calibrationTemperature = temperature; } 
    else { _newModel.calibrationTemperature = (_newModel.calibrationTemperature * (_newPointsCount - 1) + temperature) / _newPointsCount; }
    return true;
}

double CalibrationManager::calculateNewModel(const CalibrationModel& previousModel) {
    if (_newPointsCount != CALIBRATION_POINT_COUNT) return 0.0;

    double x[3] = { _newModel.points[0].voltage, _newModel.points[1].voltage, _newModel.points[2].voltage };
    double y[3] = { _newModel.points[0].value,   _newModel.points[1].value,   _newModel.points[2].value };
    double den = (x[0] - x[1]) * (x[0] - x[2]) * (x[1] - x[2]);
    if (std::abs(den) < 1e-9) return 0.0;
    _newModel.coeff_a = (x[2] * (y[1] - y[0]) + x[1] * (y[0] - y[2]) + x[0] * (y[2] - y[1])) / den;
    _newModel.coeff_b = (x[2]*x[2] * (y[0] - y[1]) + x[1]*x[1] * (y[2] - y[0]) + x[0]*x[0] * (y[1] - y[2])) / den;
    _newModel.coeff_c = (x[1] * x[2] * (x[1] - x[2]) * y[0] + x[2] * x[0] * (x[2] - x[0]) * y[1] + x[0] * x[1] * (x[0] - x[1]) * y[2]) / den;
    _newModel.isCalibrated = true;
    _newModel.lastCalibratedTimestamp = time(nullptr);

    double y_mean = (y[0] + y[1] + y[2]) / 3.0;
    double ss_tot = 0.0; double ss_res = 0.0;
    for(int i = 0; i < 3; ++i) {
        double predicted_y = (_newModel.coeff_a * pow(x[i], 2)) + (_newModel.coeff_b * x[i]) + _newModel.coeff_c;
        ss_res += pow(y[i] - predicted_y, 2);
        ss_tot += pow(y[i] - y_mean, 2);
    }
    double r_squared = (ss_tot > 1e-9) ? (1.0 - (ss_res / ss_tot)) : 1.0;
    double slope_score = 0.0;
    if (previousModel.isCalibrated && std::abs(previousModel.coeff_b) > 1e-9) {
        double slope_change = std::abs(_newModel.coeff_b - previousModel.coeff_b) / std::abs(previousModel.coeff_b);
        slope_score = std::max(0.0, 1.0 - slope_change);
    } else { slope_score = 1.0; }
    _newModel.qualityScore = ((r_squared * 100.0) * 0.6) + ((slope_score * 100.0) * 0.4);
    _newModel.qualityScore = std::max(0.0, std::min(100.0, _newModel.qualityScore));
    double total_deviation = 0.0;
    if (previousModel.isCalibrated) {
        const int steps = 100;
        double v_low = std::min({x[0], x[1], x[2]}); double v_high = std::max({x[0], x[1], x[2]});
        double step_size = (v_high - v_low) / steps;
        for (int i = 0; i <= steps; ++i) {
            double v = v_low + (i * step_size);
            double y_new = (_newModel.coeff_a * pow(v, 2)) + (_newModel.coeff_b * v) + _newModel.coeff_c;
            double y_old = (previousModel.coeff_a * pow(v, 2)) + (previousModel.coeff_b * v) + previousModel.coeff_c;
            total_deviation += std::abs(y_new - y_old);
        }
        double model_span = std::abs(std::max({y[0], y[1], y[2]}) - std::min({y[0], y[1], y[2]}));
        _newModel.sensorDrift = (model_span > 1e-9) ? (total_deviation / model_span) * 100.0 / steps : 0.0;
    } else { _newModel.sensorDrift = 0.0; }

    for (int i=0; i < CALIBRATION_POINT_COUNT; ++i) {
        if (std::abs(_newModel.points[i].value - 7.0) < 0.5) {
            _newModel.neutralVoltage = _newModel.points[i].voltage;
            break;
        }
    }
    
    if (previousModel.isCalibrated && previousModel.neutralVoltage != 0) {
        _newModel.zeroPointDrift = _newModel.neutralVoltage - previousModel.neutralVoltage;
    } else {
        _newModel.zeroPointDrift = 0.0;
    }

    return _newModel.qualityScore;
}
const CalibrationModel& CalibrationManager::getCurrentModel() const { return _currentModel; }
CalibrationModel& CalibrationManager::getMutableCurrentModel() { return _currentModel; }
const CalibrationModel& CalibrationManager::getNewModel() const { return _newModel; }
void CalibrationManager::acceptNewModel() { _currentModel = _newModel; }

/**
 * @brief --- DEFINITIVE FIX: Change signature to accept JsonObject. ---
 * This allows the model to be serialized into a nested object within a
 * larger JSON document, as required by the "Capture" snapshot feature.
 */
void CalibrationManager::serializeModel(const CalibrationModel& model, JsonObject& doc) {
    doc["isCalibrated"] = model.isCalibrated;
    doc["coeff_a"] = model.coeff_a;
    doc["coeff_b"] = model.coeff_b;
    doc["coeff_c"] = model.coeff_c;
    doc["temp"] = model.calibrationTemperature;
    doc["timestamp"] = model.lastCalibratedTimestamp;
    JsonArray points = doc.createNestedArray("points");
    for (int i = 0; i < CALIBRATION_POINT_COUNT; ++i) {
        JsonObject point = points.createNestedObject();
        point["v"] = model.points[i].voltage;
        point["val"] = model.points[i].value;
    }
    doc["quality"] = model.qualityScore;
    doc["drift"] = model.sensorDrift;
    doc["health"] = model.healthScore;
    doc["neutralV"] = model.neutralVoltage;
    doc["zpDrift"] = model.zeroPointDrift;
}


bool CalibrationManager::deserializeModel(CalibrationModel& model, JsonDocument& doc) {
    if (doc.isNull() || !doc.containsKey("isCalibrated")) return false;
    model.isCalibrated = doc["isCalibrated"];
    model.coeff_a = doc["coeff_a"];
    model.coeff_b = doc["coeff_b"];
    model.coeff_c = doc["coeff_c"];
    model.calibrationTemperature = doc["temp"] | 25.0;
    model.lastCalibratedTimestamp = doc["timestamp"] | 0;
    JsonArray points = doc["points"].as<JsonArray>();
    if (!points.isNull() && points.size() == CALIBRATION_POINT_COUNT) {
        for (int i = 0; i < CALIBRATION_POINT_COUNT; ++i) {
            model.points[i].voltage = points[i]["v"];
            model.points[i].value = points[i]["val"];
        }
    }
    model.qualityScore = doc["quality"] | 0.0;
    model.sensorDrift = doc["drift"] | 0.0;
    model.healthScore = doc["health"] | 100.0;
    model.neutralVoltage = doc["neutralV"] | 0.0;
    model.zeroPointDrift = doc["zpDrift"] | 0.0;
    return true;
}

double getTemperatureCorrectedBufferValue(double nominalValue, double temperature) {
    const float temps[] = {0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50};
    const float ph4_01[] = {4.01, 4.00, 4.00, 4.00, 4.00, 4.01, 4.01, 4.02, 4.03, 4.04, 4.06};
    const float ph6_86[] = {6.98, 6.95, 6.92, 6.90, 6.88, 6.86, 6.85, 6.84, 6.84, 6.83, 6.83};
    const float ph9_18[] = {9.46, 9.39, 9.33, 9.27, 9.22, 9.18, 9.14, 9.10, 9.07, 9.04, 9.01};
    const int numTemps = sizeof(temps) / sizeof(temps[0]);
    const float* bufferValues = nullptr;
    if (abs(nominalValue - 4.01) < 0.1) bufferValues = ph4_01;
    else if (abs(nominalValue - 6.86) < 0.1) bufferValues = ph6_86;
    else if (abs(nominalValue - 9.18) < 0.1) bufferValues = ph9_18;
    else return nominalValue;
    if (temperature <= temps[0]) return bufferValues[0];
    if (temperature >= temps[numTemps - 1]) return bufferValues[numTemps - 1];
    for (int i = 0; i < numTemps - 1; ++i) {
        if (temperature >= temps[i] && temperature < temps[i + 1]) {
            float t1 = temps[i], v1 = bufferValues[i];
            float t2 = temps[i + 1], v2 = bufferValues[i + 1];
            return v1 + (temperature - t1) * (v2 - v1) / (t2 - t1);
        }
    }
    return nominalValue;
}