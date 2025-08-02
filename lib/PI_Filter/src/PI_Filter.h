// File Path: /lib/PI_Filter/src/PI_Filter.h
// MODIFIED FILE

#ifndef PI_FILTER_H
#define PI_FILTER_H

#include <Arduino.h>
#include <vector>
#include <algorithm>
#include <numeric>

// --- NEW: Define a shared constant for history size ---
// This ensures the filter's internal buffers and the UI graph buffers are
// always synchronized, fixing the graph scaling issue.
#define FILTER_HISTORY_SIZE 128

class PI_Filter {
public:
    PI_Filter();
    double process(double rawValue);

    // Getters for KPIs
    double getFilteredValue() const;
    double getRawStandardDeviation() const;
    double getFilteredStandardDeviation() const;
    int getStabilityPercentage() const;
    bool isLocked() const;

    // Getters for historical data
    void getRawHistory(double* buffer, size_t size) const;
    void getFilteredHistory(double* buffer, size_t size) const;

    // Tunable Parameters
    int medianWindowSize;
    double settleThreshold;
    double lockSmoothing;
    double trackResponse;
    double trackAssist;

private:
    enum class FilterState { TRACKING, LOCKED };
    FilterState _currentState;

    std::vector<double> _medianHistoryBuffer; 
    
    // These buffers now act as a circular history of the last FILTER_HISTORY_SIZE data points.
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