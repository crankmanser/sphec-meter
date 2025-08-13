// File Path: /src/ui/screens/CalibrationMenuScreen.h
// NEW FILE

#ifndef CALIBRATION_MENU_SCREEN_H
#define CALIBRATION_MENU_SCREEN_H

#include "ui/StateManager.h"
#include <vector>
#include <string>

/**
 * @class CalibrationMenuScreen
 * @brief The main menu for all calibration-related tasks.
 *
 * This screen provides a centralized hub for initiating full 3-point
 * calibrations, quick 1-point health checks, and manual sensor adjustments.
 */
class CalibrationMenuScreen : public Screen {
public:
    CalibrationMenuScreen();
    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;

private:
    std::vector<std::string> _menu_items;
    std::vector<std::string> _menu_descriptions;
    int _selected_index;
};

#endif // CALIBRATION_MENU_SCREEN_H