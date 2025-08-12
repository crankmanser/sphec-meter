// File Path: /src/ui/screens/ProbeMeasurementScreen.h
// NEW FILE

#ifndef PROBE_MEASUREMENT_SCREEN_H
#define PROBE_MEASUREMENT_SCREEN_H

#include "ui/StateManager.h"
#include "ui/blocks/CalibrationCurveBlock.h" // For props struct

/**
 * @enum ProbeType
 * @brief Defines the type of probe being measured.
 */
enum class ProbeType {
    PH,
    EC
};

/**
 * @class ProbeMeasurementScreen
 * @brief A detailed, data-rich screen for displaying live probe measurements.
 *
 * This screen is generic and can be configured to display data for either a
 * pH or an EC probe. It handles probe activation/deactivation and the
 * data capture snapshot feature.
 */
class ProbeMeasurementScreen : public Screen {
public:
    ProbeMeasurementScreen();
    void onEnter(StateManager* stateManager, int probe_type_int) override;
    void onExit() override;
    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;

    // Public method for the dataTask to update the screen's data
    void updateData(double calibrated_value, double temp, int stability, double raw_mv, double filtered_mv);

    // Public method for the dataTask to know which probe is active
    ProbeType getActiveProbeType() const { return _probe_type; }

    // Public method for handling the capture request
    bool captureWasRequested();
    void clearCaptureRequest();


private:
    ProbeType _probe_type;
    bool _capture_requested;

    // --- Data to be displayed ---
    double _calibrated_value;
    double _temperature;
    int _stability_percent;
    double _raw_millivolts;
    double _filtered_millivolts;
};

#endif // PROBE_MEASUREMENT_SCREEN_H