// File Path: /src/ui/screens/pBiosMenuScreen.h
// NEW FILE

#ifndef PBIOS_MENU_SCREEN_H
#define PBIOS_MENU_SCREEN_H

#include "ui/StateManager.h"
#include <vector>
#include <string>

/**
 * @class pBiosMenuScreen
 * @brief The main menu screen for the pBios (diagnostics) mode.
 *
 * This screen provides access to low-level diagnostics and system tools.
 * It uses the standard MenuBlock for a consistent look and feel.
 */
class pBiosMenuScreen : public Screen {
public:
    pBiosMenuScreen();
    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;

private:
    std::vector<std::string> _menu_items;
    std::vector<std::string> _menu_descriptions;
    int _selected_index;
};

#endif // PBIOS_MENU_SCREEN_H