// File Path: /src/ui/screens/ProbeProfilingScreen.h
// MODIFIED FILE

#ifndef PROBE_PROFILING_SCREEN_H
#define PROBE_PROFILING_SCREEN_H

#include "ui/StateManager.h"
#include <vector>
#include <string>
#include "FilterManager.h" // Needed for PI_Filter definition

/**
 * @class ProbeProfilingScreen
 * @brief A pBIOS diagnostic screen to display a "health report" for a selected probe.
 */
class ProbeProfilingScreen : public Screen {
public:
    ProbeProfilingScreen();
    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;
    void onEnter(StateManager* stateManager) override;

    // Methods for the data task to interact with the screen
    bool isAnalyzing() const;
    uint8_t getSelectedAdcIndex() const;
    uint8_t getSelectedAdcInput() const;
    const std::string& getSelectedFilterName() const;

    /**
     * @brief --- MODIFIED: Updated to accept all "report card" KPIs. ---
     * This function is called by the data task once all live and historical
     * data for the probe profile has been aggregated.
     * @param live_r_std The freshly measured raw standard deviation of the signal.
     * @param hfFilter A snapshot of the saved High-Frequency filter parameters.
     * @param lfFilter A snapshot of the saved Low-Frequency filter parameters.
     * @param zero_point_drift The zero-point drift from the last calibration (mV).
     * @param cal_quality_score The quality score from the last calibration (%).
     * @param last_cal_timestamp The timestamp string of the last calibration.
     */
    void setAnalysisResults(double live_r_std, const PI_Filter& hfFilter, const PI_Filter& lfFilter, double zero_point_drift, double cal_quality_score, const std::string& last_cal_timestamp);

private:
    enum class ProfilingState {
        SELECT_PROBE,
        ANALYZING,
        VIEW_REPORT
    };

    void handleSelectProbeInput(const InputEvent& event);
    void handleViewReportInput(const InputEvent& event);

    ProfilingState _current_state;
    std::vector<std::string> _menu_items;
    int _selected_index;

    // --- Data for the report card ---
    // Live Data
    double _live_r_std;
    // Historical "Filter Creep" Data
    PI_Filter _hf_params_snapshot;
    PI_Filter _lf_params_snapshot;
    // --- NEW: Historical Calibration KPIs ---
    double _zero_point_drift;
    double _cal_quality_score;
    std::string _last_cal_timestamp;
};

#endif // PROBE_PROFILING_SCREEN_H