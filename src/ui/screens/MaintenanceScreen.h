// File Path: /src/ui/screens/MaintenanceScreen.h
// NEW FILE

#ifndef MAINTENANCE_SCREEN_H
#define MAINTENANCE_SCREEN_H

#include "ui/StateManager.h"
#include <vector>
#include <string>

/**
 * @class MaintenanceScreen
 * @brief The sub-menu for all pBIOS maintenance and diagnostic tasks.
 */
class MaintenanceScreen : public Screen {
public:
    MaintenanceScreen();
    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;

private:
    std::vector<std::string> _menu_items;
    std::vector<std::string> _menu_descriptions;
    int _selected_index;
};

#endif // MAINTENANCE_SCREEN_H