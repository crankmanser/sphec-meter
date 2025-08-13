// File Path: /src/ui/screens/AutoTuneSubMenuScreen.h
// MODIFIED FILE

#ifndef AUTO_TUNE_SUB_MENU_SCREEN_H
#define AUTO_TUNE_SUB_MENU_SCREEN_H

#include "ui/StateManager.h"
#include <vector>
#include <string>

// Forward declare AdcManager to avoid circular dependencies
class AdcManager;

/**
 * @class AutoTuneSubMenuScreen
 * @brief The sub-menu for the auto-tuner, providing access to the main
 * guided wizard and individual diagnostic steps.
 * --- DEFINITIVE REFACTOR: This screen no longer manages hardware state. ---
 */
class AutoTuneSubMenuScreen : public Screen {
public:
    // --- DEFINITIVE REFACTOR: Constructor no longer needs the AdcManager ---
    AutoTuneSubMenuScreen();
    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;

private:
    // --- DEFINITIVE REFACTOR: AdcManager pointer is removed ---
    std::vector<std::string> _menu_items;
    std::vector<std::string> _menu_descriptions;
    int _selected_index;
};

#endif // AUTO_TUNE_SUB_MENU_SCREEN_H