// File Path: /src/ui/screens/CalibrationWizardScreen.h
// MODIFIED FILE

#ifndef CALIBRATION_WIZARD_SCREEN_H
#define CALIBRATION_WIZARD_SCREEN_H

#include "ui/StateManager.h"
#include "ProbeMeasurementScreen.h" // For ProbeType enum

/**
 * @class CalibrationWizardScreen
 * @brief A multi-step guided wizard for performing a 3-point probe calibration.
 */
class CalibrationWizardScreen : public Screen {
public:
    CalibrationWizardScreen();
    void onEnter(StateManager* stateManager, int context = 0) override;
    // --- NEW: Add the onExit method to deactivate the probe ---
    void onExit() override;
    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;

    // Public methods for the dataTask to interact with the screen
    ProbeType getProbeType() const { return _probe_type; }
    int getCurrentStep() const { return _current_step; }
    bool isMeasuring() const;
    bool isCalculating() const;
    bool saveWasRequested();
    void clearSaveRequest();
    void setLiveStability(int percent);
    void setResults(double quality_score, double sensor_drift);

private:
    enum class WizardState {
        INTRODUCTION,
        MEASURE_POINT,
        CALCULATING,
        VIEW_RESULTS
    };

    void handleIntroInput(const InputEvent& event);
    void handleMeasurePointInput(const InputEvent& event);
    void handleResultsInput(const InputEvent& event);

    ProbeType _probe_type;
    WizardState _wizard_state;
    int _current_step; 
    int _live_stability_percent;
    bool _save_requested;
    
    double _result_quality_score;
    double _result_sensor_drift;
};

#endif // CALIBRATION_WIZARD_SCREEN_H