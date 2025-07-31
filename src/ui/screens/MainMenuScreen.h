// File Path: /src/ui/screens/MainMenuScreen.h
// NEW FILE

#ifndef MAIN_MENU_SCREEN_H
#define MAIN_MENU_SCREEN_H

#include "ui/StateManager.h"
#include <vector>
#include <string>

class MainMenuScreen : public Screen {
public:
    MainMenuScreen();
    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;

private:
    std::vector<std::string> _menu_items;
    std::vector<std::string> _menu_descriptions;
    int _selected_index;
};

#endif // MAIN_MENU_SCREEN_H