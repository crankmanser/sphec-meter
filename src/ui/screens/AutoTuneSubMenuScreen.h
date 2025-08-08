// File Path: /src/ui/screens/AutoTuneSubMenuScreen.h
// MODIFIED FILE

#ifndef AUTO_TUNE_SUB_MENU_SCREEN_H
#define AUTO_TUNE_SUB_MENU_SCREEN_H

#include "ui/StateManager.h"
#include <vector>
#include <string>

class AutoTuneSubMenuScreen : public Screen {
public:
    AutoTuneSubMenuScreen();
    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;

private:
    std::vector<std::string> _menu_items;
    // --- DEFINITIVE FIX: Declare the missing member variable ---
    std::vector<std::string> _menu_descriptions;
    int _selected_index;
};

#endif // AUTO_TUNE_SUB_MENU_SCREEN_H