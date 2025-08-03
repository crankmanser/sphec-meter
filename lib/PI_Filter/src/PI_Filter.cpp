// File Path: /lib/PI_Filter/src/PI_Filter.cpp
// MODIFIED FILE

#include "PI_Filter.h"
#include <cmath> 

// ... (constructor, process, and updateBuffers are unchanged) ...
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
        _rawStdDev = 0; _filteredStdDev = 0; _stabilityPercent = 0;
        return;
    }

    double rawSum = std::accumulate(_rawBuffer.begin(), _rawBuffer.end(), 0.0);
    double rawMean = rawSum / _rawBuffer.size();
    if (isnan(rawMean)) { rawMean = 0; }
    double rawSqSum = std::inner_product(_rawBuffer.begin(), _rawBuffer.end(), _rawBuffer.begin(), 0.0);
    double rawVariance = rawSqSum / _rawBuffer.size() - rawMean * rawMean;
    _rawStdDev = (rawVariance > 0 && !isnan(rawVariance)) ? std::sqrt(rawVariance) : 0.0;

    double filteredSum = std::accumulate(_filteredBuffer.begin(), _filteredBuffer.end(), 0.0);
    double filteredMean = filteredSum / _filteredBuffer.size();
    if (isnan(filteredMean)) { filteredMean = 0; }
    double filteredSqSum = std::inner_product(_filteredBuffer.begin(), _filteredBuffer.end(), _filteredBuffer.begin(), 0.0);
    double filteredVariance = filteredSqSum / _filteredBuffer.size() - filteredMean * filteredMean;
    _filteredStdDev = (filteredVariance > 0 && !isnan(filteredVariance)) ? std::sqrt(filteredVariance) : 0.0;

    if (_rawStdDev > 0.00001) {
        double improvement = 1.0 - (_filteredStdDev / _rawStdDev);
        _stabilityPercent = (int)constrain(improvement * 100.0, 0.0, 100.0);
    } else {
        _stabilityPercent = 100;
    }
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