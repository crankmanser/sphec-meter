// File Path: /lib/PI_Filter/src/PI_Filter.cpp

#include "PI_Filter.h"
#include <cmath> // For std::sqrt

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
    _rawBuffer.reserve(50);
    _filteredBuffer.reserve(50);
}

double PI_Filter::process(double rawValue) {
    // --- FIX: Stage 1 now uses its own dedicated sliding window buffer ---
    _medianHistoryBuffer.push_back(rawValue);

    // Keep the median buffer at the specified window size
    if (_medianHistoryBuffer.size() > (size_t)medianWindowSize) {
        _medianHistoryBuffer.erase(_medianHistoryBuffer.begin());
    }

    // Create a temporary copy for sorting to find the median value
    std::vector<double> sortedMedianBuffer = _medianHistoryBuffer;
    std::sort(sortedMedianBuffer.begin(), sortedMedianBuffer.end());
    double medianValue = sortedMedianBuffer[sortedMedianBuffer.size() / 2];
    
    // --- State Transition Logic ---
    double change = std::abs(medianValue - _filteredValue);
    if (change < settleThreshold) {
        _currentState = FilterState::LOCKED;
    } else {
        _currentState = FilterState::TRACKING;
    }

    // --- Stage 2: Stateful PI Smoothing ---
    if (_currentState == FilterState::LOCKED) {
        _filteredValue = (_filteredValue * (1.0 - lockSmoothing)) + (medianValue * lockSmoothing);
        _integralTerm = 0;
    } else { // TRACKING State
        double error = medianValue - _filteredValue;
        _integralTerm += error * trackAssist;
        _filteredValue = (_filteredValue * (1.0 - trackResponse)) + (medianValue * trackResponse) + _integralTerm;
    }

    // --- Update Statistics ---
    updateBuffers(rawValue, _filteredValue);
    calculateStatistics();

    return _filteredValue;
}

// --- Helper methods are unchanged ---

void PI_Filter::updateBuffers(double rawValue, double filteredValue) {
    _rawBuffer.push_back(rawValue);
    _filteredBuffer.push_back(filteredValue);

    if (_rawBuffer.size() > 50) {
        _rawBuffer.erase(_rawBuffer.begin());
    }
    if (_filteredBuffer.size() > 50) {
        _filteredBuffer.erase(_filteredBuffer.begin());
    }
}

void PI_Filter::calculateStatistics() {
    if (_rawBuffer.size() < 2) {
        _rawStdDev = 0;
        _filteredStdDev = 0;
        _stabilityPercent = 0;
        return;
    }

    double rawSum = std::accumulate(_rawBuffer.begin(), _rawBuffer.end(), 0.0);
    double rawMean = rawSum / _rawBuffer.size();
    double rawSqSum = std::inner_product(_rawBuffer.begin(), _rawBuffer.end(), _rawBuffer.begin(), 0.0);
    _rawStdDev = std::sqrt(rawSqSum / _rawBuffer.size() - rawMean * rawMean);

    double filteredSum = std::accumulate(_filteredBuffer.begin(), _filteredBuffer.end(), 0.0);
    double filteredMean = filteredSum / _filteredBuffer.size();
    double filteredSqSum = std::inner_product(_filteredBuffer.begin(), _filteredBuffer.end(), _filteredBuffer.begin(), 0.0);
    _filteredStdDev = std::sqrt(filteredSqSum / _filteredBuffer.size() - filteredMean * filteredMean);

    if (_rawStdDev > 0.0001) {
        double improvement = 1.0 - (_filteredStdDev / _rawStdDev);
        _stabilityPercent = (int)constrain(improvement * 100.0, 0.0, 100.0);
    } else {
        _stabilityPercent = 100;
    }
}

double PI_Filter::getFilteredValue() const { return _filteredValue; }
double PI_Filter::getRawStandardDeviation() const { return _rawStdDev; }
double PI_Filter::getFilteredStandardDeviation() const { return _filteredStdDev; }
int PI_Filter::getStabilityPercentage() const { return _stabilityPercent; }
bool PI_Filter::isLocked() const { return _currentState == FilterState::LOCKED; }