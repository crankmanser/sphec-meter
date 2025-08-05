// File Path: /lib/PI_Filter/src/PI_Filter.cpp
// MODIFIED FILE

#include "PI_Filter.h"
#include <cmath> 
#include <numeric>
// --- FIX: Reverted to a simple include. The build flag will handle the path. ---
#include "DebugConfig.h"

// ... (The rest of the file is correct and remains unchanged) ...
// ... (constructor, process, updateBuffers) ...
PI_Filter::PI_Filter() :
    medianWindowSize(5),
    settleThreshold(0.1),
    lockSmoothing(0.02),
    trackResponse(0.8),
    trackAssist(0.005),
    _currentState(FilterState::TRACKING),
    _filteredValue(0.0),
    _integralTerm(0.0),
    _rawStdDev(0.0),
    _filteredStdDev(0.0),
    _stabilityPercent(0)
{
    _rawBuffer.reserve(FILTER_HISTORY_SIZE);
    _filteredBuffer.reserve(FILTER_HISTORY_SIZE);
}
double PI_Filter::process(double rawValue) {
    if (isnan(rawValue)) { return _filteredValue; }
    _medianHistoryBuffer.push_back(rawValue);
    if (_medianHistoryBuffer.size() > (size_t)medianWindowSize) {
        _medianHistoryBuffer.erase(_medianHistoryBuffer.begin());
    }
    std::vector<double> sortedMedianBuffer = _medianHistoryBuffer;
    std::sort(sortedMedianBuffer.begin(), sortedMedianBuffer.end());
    double medianValue = sortedMedianBuffer[sortedMedianBuffer.size() / 2];
    double change = std::abs(medianValue - _filteredValue);
    if (change < settleThreshold) {
        _currentState = FilterState::LOCKED;
    } else {
        _currentState = FilterState::TRACKING;
    }
    if (_currentState == FilterState::LOCKED) {
        _filteredValue = (_filteredValue * (1.0 - lockSmoothing)) + (medianValue * lockSmoothing);
        _integralTerm = 0;
    } else { 
        double error = medianValue - _filteredValue;
        _integralTerm += error * trackAssist;
        _filteredValue = (_filteredValue * (1.0 - trackResponse)) + (medianValue * trackResponse) + _integralTerm;
    }
    updateBuffers(rawValue, _filteredValue);
    calculateStatistics();
    return _filteredValue;
}
void PI_Filter::updateBuffers(double rawValue, double filteredValue) {
    _rawBuffer.push_back(rawValue);
    _filteredBuffer.push_back(filteredValue);
    if (_rawBuffer.size() > FILTER_HISTORY_SIZE) {
        _rawBuffer.erase(_rawBuffer.begin());
    }
    if (_filteredBuffer.size() > FILTER_HISTORY_SIZE) {
        _filteredBuffer.erase(_filteredBuffer.begin());
    }
}

void PI_Filter::calculateStatistics() {
    if (_rawBuffer.size() < 2) {
        _rawStdDev = 0.0; _filteredStdDev = 0.0; _stabilityPercent = 100;
        return;
    }

    LOG_FILTER("--- START CALC (Buffer Size: %d) ---", _rawBuffer.size());

    // --- Raw Standard Deviation ---
    double rawSum = 0.0;
    for(double val : _rawBuffer) { rawSum += val; }
    double rawMean = rawSum / _rawBuffer.size();
    double rawSumSqDiff = 0.0;
    for(double val : _rawBuffer) { rawSumSqDiff += (val - rawMean) * (val - rawMean); }
    double rawVariance = rawSumSqDiff / _rawBuffer.size();
    
    LOG_FILTER("RAW: Sum=%.4f, Mean=%.4f, SumSqDiff=%.4f, Variance=%.10f", rawSum, rawMean, rawSumSqDiff, rawVariance);
    
    if (rawVariance < 0.0) {
        LOG_FILTER("!!! RAW VARIANCE IS NEGATIVE !!!");
    }
    _rawStdDev = (rawVariance > 0.0) ? std::sqrt(rawVariance) : 0.0;
    LOG_FILTER("--> RAW StdDev = %.10f", _rawStdDev);

    // --- Filtered Standard Deviation ---
    double filteredSum = 0.0;
    for(double val : _filteredBuffer) { filteredSum += val; }
    double filteredMean = filteredSum / _filteredBuffer.size();
    double filteredSumSqDiff = 0.0;
    for(double val : _filteredBuffer) { filteredSumSqDiff += (val - filteredMean) * (val - filteredMean); }
    double filteredVariance = filteredSumSqDiff / _filteredBuffer.size();
    
    LOG_FILTER("FIL: Sum=%.4f, Mean=%.4f, SumSqDiff=%.4f, Variance=%.10f", filteredSum, filteredMean, filteredSumSqDiff, filteredVariance);

    if (filteredVariance < 0.0) {
        LOG_FILTER("!!! FILT VARIANCE IS NEGATIVE !!!");
    }
    _filteredStdDev = (filteredVariance > 0.0) ? std::sqrt(filteredVariance) : 0.0;
    LOG_FILTER("--> FIL StdDev = %.10f", _filteredStdDev);

    // --- Stability Percentage ---
    if (_rawStdDev > 1e-9) {
        double improvement = 1.0 - (_filteredStdDev / _rawStdDev);
        _stabilityPercent = (int)constrain(improvement * 100.0, 0.0, 100.0);
    } else {
        _stabilityPercent = 100;
    }
    LOG_FILTER("--- END CALC (Stab=%d%%) ---", _stabilityPercent);
}

void PI_Filter::getRawHistory(double* buffer, size_t size) const {
    size_t historySize = _rawBuffer.size();
    size_t padding = (size > historySize) ? (size - historySize) : 0;
    for (size_t i = 0; i < padding; ++i) { buffer[i] = (historySize > 0) ? _rawBuffer[0] : 0.0; }
    for (size_t i = 0; i < historySize; ++i) { buffer[padding + i] = _rawBuffer[i]; }
}
void PI_Filter::getFilteredHistory(double* buffer, size_t size) const {
    size_t historySize = _filteredBuffer.size();
    size_t padding = (size > historySize) ? (size - historySize) : 0;
    for (size_t i = 0; i < padding; ++i) { buffer[i] = (historySize > 0) ? _filteredBuffer[0] : 0.0; }
    for (size_t i = 0; i < historySize; ++i) { buffer[padding + i] = _filteredBuffer[i]; }
}
double PI_Filter::getFilteredValue() const { return _filteredValue; }
double PI_Filter::getRawStandardDeviation() const { return _rawStdDev; }
double PI_Filter::getFilteredStandardDeviation() const { return _filteredStdDev; }
int PI_Filter::getStabilityPercentage() const { return _stabilityPercent; }
bool PI_Filter::isLocked() const { return _currentState == FilterState::LOCKED; }