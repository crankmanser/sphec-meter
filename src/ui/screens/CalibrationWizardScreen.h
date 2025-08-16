// File Path: /src/ui/screens/CalibrationWizardScreen.h
// MODIFIED FILE

#ifndef CALIBRATION_WIZARD_SCREEN_H
#define CALIBRATION_WIZARD_SCREEN_H

#include "ui/StateManager.h"
#include "ProbeMeasurementScreen.h" // For ProbeType enum

/**
 * @class CalibrationWizardScreen
 * @brief A multi-step guided wizard for performing a 3-point probe calibration.
 * @version 3.1.11
 */
class CalibrationWizardScreen : public Screen {
public:
    CalibrationWizardScreen();
    void onEnter(StateManager* stateManager, int context = 0) override;
    void onExit() override;
    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;

    // --- Public methods for the dataTask to interact with the screen ---
    ProbeType getProbeType() const { return _probe_type; }
    int getCurrentStep() const { return _current_step; }

    // State query methods
    bool isMeasuring() const;
    bool isCalculating() const;

    // --- NEW: Request/clear methods for point capture ---
    bool pointCaptureWasRequested();
    void clearPointCaptureRequest();

    // Request/clear methods for saving the final model
    bool saveWasRequested();
    void clearSaveRequest();

    // Data update methods from the backend
    void setLiveStability(int percent);
    void setResults(double quality_score, double sensor_drift);

    // --- NEW: Public methods for the dataTask to control wizard flow ---
    void advanceToNextStep();
    void transitionToCalculating();


private:
    enum class WizardState {
        INTRODUCTION,
        MEASURE_POINT,
        CALCULATING,
        VIEW_RESULTS
    };

    // Input handlers for each state
    void handleIntroInput(const InputEvent& event);
    void handleMeasurePointInput(const InputEvent& event);
    void handleResultsInput(const InputEvent& event);

    // --- Member Variables ---
    ProbeType _probe_type;
    WizardState _wizard_state;
    int _current_step;
    int _live_stability_percent;

    // --- NEW: Flag for signaling a point capture to the dataTask ---
    bool _point_capture_requested;
    bool _save_requested;

    // Data for the results screen
    double _result_quality_score;
    double _result_sensor_drift;
};

#endif // CALIBRATION_WIZARD_SCREEN_H