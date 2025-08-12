// File Path: /src/ui/screens/MeasureMenuScreen.h
// NEW FILE

#ifndef MEASURE_MENU_SCREEN_H
#define MEASURE_MENU_SCREEN_H

#include "ui/StateManager.h"
#include <vector>
#include <string>

/**
 * @class MeasureMenuScreen
 * @brief A sub-menu for selecting which type of measurement to perform.
 *
 * This screen acts as a gateway to the dedicated, data-rich measurement
 * screens for each primary probe type.
 */
class MeasureMenuScreen : public Screen {
public:
    MeasureMenuScreen();
    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;

private:
    std::vector<std::string> _menu_items;
    std::vector<std::string> _menu_descriptions;
    int _selected_index;
};

#endif // MEASURE_MENU_SCREEN_H