// File Path: /lib/PI_Filter/src/PI_Filter.h
// MODIFIED FILE

#ifndef PI_FILTER_H
#define PI_FILTER_H

#include <Arduino.h>
#include <vector>
#include <algorithm>
#include <numeric>

#define FILTER_HISTORY_SIZE 128

class PI_Filter {
public:
    PI_Filter();

    /**
     * @brief --- NEW: Copy Constructor (Rule of Three) ---
     * Ensures a deep copy is performed when a PI_Filter object is copied,
     * preventing heap corruption and double-free errors.
     */
    PI_Filter(const PI_Filter& other);

    /**
     * @brief --- NEW: Copy Assignment Operator (Rule of Three) ---
     * Ensures a deep copy is performed when a PI_Filter object is assigned,
     * preventing memory leaks and heap corruption.
     */
    PI_Filter& operator=(const PI_Filter& other);

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