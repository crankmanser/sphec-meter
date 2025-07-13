// src/presentation/screens/main_menu/MainMenuScreen.h
// NEW FILE
#pragma once

#include "presentation/screens/Screen.h"
#include <vector>
#include <string>

class MainMenuScreen : public Screen {
public:
    MainMenuScreen();
    void handleInput(const InputEvent& event) override;
    UIRenderProps getRenderProps() override;

private:
    std::vector<std::string> _menu_items;
    std::vector<std::string> _menu_descriptions;
    int _selected_index;
};