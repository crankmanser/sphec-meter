// File Path: /src/ui/screens/ShutdownScreen.h
// NEW FILE

#ifndef SHUTDOWN_SCREEN_H
#define SHUTDOWN_SCREEN_H

#include "ui/StateManager.h"
#include <vector>
#include <string>

/**
 * @class ShutdownScreen
 * @brief The sub-menu for confirming shutdown options.
 */
class ShutdownScreen : public Screen {
public:
    ShutdownScreen();
    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;

private:
    std::vector<std::string> _menu_items;
    int _selected_index;
};

#endif // SHUTDOWN_SCREEN_H