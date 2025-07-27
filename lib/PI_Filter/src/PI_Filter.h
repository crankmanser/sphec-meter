// File Path: /lib/PI_Filter/src/PI_Filter.h

#ifndef PI_FILTER_H
#define PI_FILTER_H

#include <Arduino.h>
#include <vector>
#include <algorithm>
#include <numeric>

class PI_Filter {
public:
    PI_Filter();
    double process(double rawValue);

    // Getters
    double getFilteredValue() const;
    double getRawStandardDeviation() const;
    double getFilteredStandardDeviation() const;
    int getStabilityPercentage() const;
    bool isLocked() const;

    // Tunable Parameters
    int medianWindowSize;
    double settleThreshold;
    double lockSmoothing;
    double trackResponse;
    double trackAssist;

private:
    enum class FilterState { TRACKING, LOCKED };
    FilterState _currentState;

    // --- FIX: Added a dedicated buffer for the median filter's sliding window ---
    std::vector<double> _medianHistoryBuffer; 
    
    std::vector<double> _rawBuffer;      
    std::vector<double> _filteredBuffer;
    double _filteredValue;
    double _integralTerm;
    double _rawStdDev;
    double _filteredStdDev;
    int _stabilityPercent;

    void updateBuffers(double rawValue, double filteredValue);
    void calculateStatistics();
};

#endif // PI_FILTER_H