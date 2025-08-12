// File Path: /src/ui/screens/PowerOffScreen.h
// NEW FILE

#ifndef POWER_OFF_SCREEN_H
#define POWER_OFF_SCREEN_H

#include "ui/StateManager.h"

/**
 * @class PowerOffScreen
 * @brief A final, static screen indicating it is safe to power off the device.
 *
 * This screen is non-interactive and serves as a clear signal to the user
 * that all data has been saved and the shutdown process is complete.
 */
class PowerOffScreen : public Screen {
public:
    PowerOffScreen();
    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;
};

#endif // POWER_OFF_SCREEN_H