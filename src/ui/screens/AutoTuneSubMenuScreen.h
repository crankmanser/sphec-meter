// File Path: /src/ui/screens/AutoTuneSubMenuScreen.h
// MODIFIED FILE

#ifndef AUTO_TUNE_SUB_MENU_SCREEN_H
#define AUTO_TUNE_SUB_MENU_SCREEN_H

#include "ui/StateManager.h"
#include <vector>
#include <string>

// Forward declare AdcManager to avoid circular dependencies
class AdcManager;

class AutoTuneSubMenuScreen : public Screen {
public:
    /**
     * @brief --- NEW: The constructor now accepts the AdcManager. ---
     * This is required so the screen can activate the probe before tuning.
     * @param adcManager A pointer to the global AdcManager instance.
     */
    AutoTuneSubMenuScreen(AdcManager* adcManager);
    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;

private:
    AdcManager* _adcManager; // Pointer to the ADC manager
    std::vector<std::string> _menu_items;
    std::vector<std::string> _menu_descriptions;
    int _selected_index;
};

#endif // AUTO_TUNE_SUB_MENU_SCREEN_H