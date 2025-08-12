// File Path: /src/ui/screens/LightSensorScreen.h
// NEW FILE

#ifndef LIGHT_SENSOR_SCREEN_H
#define LIGHT_SENSOR_SCREEN_H

#include "ui/StateManager.h"

/**
 * @class LightSensorScreen
 * @brief A placeholder screen for the Light Sensor feature.
 */
class LightSensorScreen : public Screen {
public:
    LightSensorScreen();
    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;
};

#endif // LIGHT_SENSOR_SCREEN_H