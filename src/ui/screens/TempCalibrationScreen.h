// File Path: /src/ui/screens/TempCalibrationScreen.h
// NEW FILE

#ifndef TEMP_CALIBRATION_SCREEN_H
#define TEMP_CALIBRATION_SCREEN_H

#include "ui/StateManager.h"

/**
 * @class TempCalibrationScreen
 * @brief A screen for manually calibrating the temperature sensors.
 */
class TempCalibrationScreen : public Screen {
public:
    TempCalibrationScreen();
    void onEnter(StateManager* stateManager, int context = 0) override;
    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;

private:
    float _probe_temp_live;
    float _ambient_temp_live;
    float _ref_temp; // The value the user is editing
    bool _is_editing;
};

#endif // TEMP_CALIBRATION_SCREEN_H