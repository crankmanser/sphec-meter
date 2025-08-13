// File Path: /src/ui/screens/AutoTuneSubMenuScreen.cpp
// MODIFIED FILE

#include "AutoTuneSubMenuScreen.h"
#include "ui/StateManager.h" 
#include "ui/UIManager.h" 
#include "pBiosContext.h" 
#include "AdcManager.h" // Include the full AdcManager header

extern PBiosContext pBiosContext;

/**
 * @brief --- MODIFIED: The constructor now initializes the AdcManager pointer. ---
 */
AutoTuneSubMenuScreen::AutoTuneSubMenuScreen(AdcManager* adcManager) : 
    _adcManager(adcManager),
    _selected_index(0) 
{
    _menu_items.push_back("Tuner Wizard");
    _menu_items.push_back("Signal Profile");
    _menu_items.push_back("HF Optimization");
    _menu_items.push_back("LF Optimization");
    _menu_items.push_back("Probe Correction");

    _menu_descriptions.push_back("Run the full, guided tuning process.");
    _menu_descriptions.push_back("Run a standalone signal analysis.");
    _menu_descriptions.push_back("Run only the HF stage optimization.");
    _menu_descriptions.push_back("Run only the LF stage optimization.");
    _menu_descriptions.push_back("Apply corrections from probe profile.");
}

void AutoTuneSubMenuScreen::handleInput(const InputEvent& event) {
    if (event.type == InputEventType::ENCODER_INCREMENT) {
        if (_selected_index < _menu_items.size() - 1) _selected_index++;
    } else if (event.type == InputEventType::ENCODER_DECREMENT) {
        if (_selected_index > 0) _selected_index--;
    } else if (event.type == InputEventType::BTN_DOWN_PRESS) {
        const std::string& selected_item = _menu_items[_selected_index];
        
        if (selected_item == "Tuner Wizard") {
            if (_stateManager && pBiosContext.selectedFilter && _adcManager) {
                // --- DEFINITIVE FIX: Wake up the correct probe before tuning. ---
                // This ensures the GuidedTuningEngine analyzes a live signal
                // instead of a stream of zeros from a dormant probe.
                _adcManager->setProbeState(pBiosContext.selectedAdcIndex, ProbeState::ACTIVE);

                pBiosContext.hf_params_snapshot = *pBiosContext.selectedFilter->getFilter(0);
                pBiosContext.lf_params_snapshot = *pBiosContext.selectedFilter->getFilter(1);
                
                _stateManager->changeState(ScreenState::AUTO_TUNE_RUNNING);
            }
        }
    } else if (event.type == InputEventType::BTN_BACK_PRESS) {
        if (_stateManager) _stateManager->changeState(ScreenState::LIVE_FILTER_TUNING);
    }
}

void AutoTuneSubMenuScreen::getRenderProps(UIRenderProps* props_to_fill) {
    if (_selected_index < _menu_descriptions.size()) {
        props_to_fill->oled_top_props.line1 = _menu_descriptions[_selected_index];
    }
    
    props_to_fill->oled_middle_props.menu_props.is_enabled = true;
    props_to_fill->oled_middle_props.menu_props.items = _menu_items;
    props_to_fill->oled_middle_props.menu_props.selected_index = _selected_index;

    props_to_fill->oled_bottom_props.line1 = "pBios > Tuning > Auto Tune";

    props_to_fill->button_props.back_text = "Back";
    props_to_fill->button_props.down_text = "Run";
}