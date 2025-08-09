// File Path: /src/ui/screens/LiveFilterTuningScreen.cpp
// MODIFIED FILE

#include "LiveFilterTuningScreen.h"
#include "pBiosContext.h"
#include "ConfigManager.h"
#include "AdcManager.h"
#include "CalibrationManager.h"
#include "TempManager.h"
#include <stdio.h>
#include <Arduino.h>

extern ConfigManager configManager;
extern FilterManager phFilter, ecFilter;

LiveFilterTuningScreen::LiveFilterTuningScreen(AdcManager* adcManager, PBiosContext* context, CalibrationManager* phCal, CalibrationManager* ecCal, TempManager* tempManager) :
    _adcManager(adcManager),
    _context(context),
    _phCalManager(phCal),
    _ecCalManager(ecCal),
    _tempManager(tempManager),
    _is_in_manual_tune_mode(false),
    _selected_index(0),
    _is_compare_mode_active(false),
    _calibrated_value(NAN)
{
    for (int i = 0; i < GRAPH_DATA_POINTS; ++i) { 
        _hf_raw_buffer[i] = 0.0; _hf_filtered_buffer[i] = 0.0; 
        _lf_filtered_buffer[i] = 0.0; _ghost_lf_filtered_buffer[i] = 0.0;
    }

    _hub_menu_items.push_back("Auto Tune");
    _hub_menu_items.push_back("Manual Tune");
    _hub_menu_items.push_back("Save Tune");
    _hub_menu_items.push_back("Restore Tune");
    _hub_menu_items.push_back("Exit");

    _hub_menu_descriptions.push_back("Run the automated tuning wizard.");
    _hub_menu_descriptions.push_back("Manually adjust all filter parameters.");
    _hub_menu_descriptions.push_back("Save the current settings to the SD card.");
    _hub_menu_descriptions.push_back("Load the last saved settings.");
    _hub_menu_descriptions.push_back("Return to the filter selection screen.");
}

void LiveFilterTuningScreen::onEnter(StateManager* stateManager) {
    Screen::onEnter(stateManager);
    _is_in_manual_tune_mode = false; 
    _selected_index = 0;
    _is_compare_mode_active = false;
}

void LiveFilterTuningScreen::handleInput(const InputEvent& event) {
    if (_is_in_manual_tune_mode) {
        if (_stateManager) {
            Screen* editScreen = _stateManager->getScreen(ScreenState::PARAMETER_EDIT);
            if (editScreen) editScreen->handleInput(event);
        }
    } else {
        handleHubMenuInput(event);
    }
}

/**
 * @brief --- DEFINITIVE FIX: The main render function now correctly assembles the hub view. ---
 * It calls a helper to get the base graph layout and then overwrites the necessary
 * sections with the hub's specific content (help text and menu).
 */
void LiveFilterTuningScreen::getRenderProps(UIRenderProps* props_to_fill) {
    *props_to_fill = UIRenderProps();

    // 1. Get the base layout with graphs and KPIs for all screens
    getManualTuneRenderProps(props_to_fill);

    // 2. Overwrite the TOP OLED's text with contextual help for the hub menu
    if (_selected_index < _hub_menu_descriptions.size()) {
        props_to_fill->oled_top_props.line1 = _hub_menu_descriptions[_selected_index];
    }
    
    // 3. Overwrite the MIDDLE OLED with the hub menu
    OledProps& mid_props = props_to_fill->oled_middle_props;
    mid_props = OledProps(); // Clear any graph data from the middle
    mid_props.menu_props.is_enabled = true;
    mid_props.menu_props.items = _hub_menu_items;
    mid_props.menu_props.selected_index = _selected_index;

    // 4. Set the correct button prompts for the hub
    props_to_fill->button_props.down_text = "Select";
}


void LiveFilterTuningScreen::update() {
    if (!_context || !_context->selectedFilter) return;
    PI_Filter* hfFilter = _context->selectedFilter->getFilter(0);
    PI_Filter* lfFilter = _context->selectedFilter->getFilter(1);
    if (hfFilter) {
        hfFilter->getRawHistory(_hf_raw_buffer, GRAPH_DATA_POINTS);
        hfFilter->getFilteredHistory(_hf_filtered_buffer, GRAPH_DATA_POINTS);
        _hf_r_std = hfFilter->getRawStandardDeviation();
        _hf_f_std = hfFilter->getFilteredStandardDeviation();
        _hf_stab_percent = hfFilter->getStabilityPercentage();
    }
    if (lfFilter) {
        lfFilter->getFilteredHistory(_lf_filtered_buffer, GRAPH_DATA_POINTS);
        _lf_r_std = hfFilter ? hfFilter->getFilteredStandardDeviation() : 0.0;
        _lf_f_std = lfFilter->getFilteredStandardDeviation();
        _lf_stab_percent = lfFilter->getStabilityPercentage();
    }
    if (lfFilter) {
        double filtered_voltage = lfFilter->getFilteredValue();
        float temp = _tempManager->getProbeTemp();
        if (_context->selectedFilter == &phFilter) {
            _calibrated_value = _phCalManager->getCompensatedValue(_phCalManager->getCalibratedValue(filtered_voltage), temp, false);
        } else if (_context->selectedFilter == &ecFilter) {
            _calibrated_value = _ecCalManager->getCompensatedValue(_ecCalManager->getCalibratedValue(filtered_voltage), temp, true);
        }
    }
}

void LiveFilterTuningScreen::handleHubMenuInput(const InputEvent& event) {
    if (event.type == InputEventType::ENCODER_INCREMENT) {
        if (_selected_index < _hub_menu_items.size() - 1) _selected_index++;
    } else if (event.type == InputEventType::ENCODER_DECREMENT) {
        if (_selected_index > 0) _selected_index--;
    } else if (event.type == InputEventType::BTN_DOWN_PRESS) {
        const std::string& selected_item = _hub_menu_items[_selected_index];
        
        if (selected_item == "Auto Tune") { if (_stateManager) _stateManager->changeState(ScreenState::AUTO_TUNE_SUB_MENU); }
        else if (selected_item == "Manual Tune") {
            _is_in_manual_tune_mode = true;
            if (_stateManager) _stateManager->changeState(ScreenState::PARAMETER_EDIT);
        }
        else if (selected_item == "Save Tune") {
            if (_context && _context->selectedFilter) {
                configManager.saveFilterSettings(*_context->selectedFilter, _context->selectedFilterName.c_str(), true);
                configManager.saveFilterSettings(*_context->selectedFilter, _context->selectedFilterName.c_str(), false);
            }
        }
        else if (selected_item == "Restore Tune") {
            if (_context && _context->selectedFilter) {
                configManager.loadFilterSettings(*_context->selectedFilter, _context->selectedFilterName.c_str(), true);
            }
        }
        else if (selected_item == "Exit") { if (_stateManager) _stateManager->changeState(ScreenState::FILTER_SELECTION); }
    }
}

/**
 * @brief This helper function populates the UIRenderProps with all the data
 * needed to draw the graphs and their associated labels on the top and bottom screens.
 * It is called by both the hub and the parameter edit screen.
 */
void LiveFilterTuningScreen::getManualTuneRenderProps(UIRenderProps* props_to_fill) {
    static char hf_top[40], hf_bl[10], hf_br[20], lf_top[40], lf_bl[10], lf_br[20], r_buf[10], f_buf[10];
    
    // --- Top OLED: HF Graph and KPIs ---
    dtostrf(_hf_r_std, 4, 3, r_buf); dtostrf(_hf_f_std, 4, 3, f_buf);
    snprintf(hf_top, sizeof(hf_top), "R:%s F:%s", r_buf, f_buf);
    snprintf(hf_bl, sizeof(hf_bl), "HF"); snprintf(hf_br, sizeof(hf_br), "Stab:%d%%", _hf_stab_percent);
    props_to_fill->oled_top_props.graph_props.is_enabled = true;
    props_to_fill->oled_top_props.graph_props.pre_filter_data = _hf_raw_buffer;
    props_to_fill->oled_top_props.graph_props.post_filter_data = _hf_filtered_buffer;
    props_to_fill->oled_top_props.graph_props.top_left_label = hf_top;
    props_to_fill->oled_top_props.graph_props.bottom_left_label = hf_bl;
    props_to_fill->oled_top_props.graph_props.bottom_right_label = hf_br;
    
    // --- Bottom OLED: LF Graph, KPIs, and Calibrated Value ---
    dtostrf(_lf_r_std, 4, 3, r_buf); dtostrf(_lf_f_std, 4, 3, f_buf);
    snprintf(lf_top, sizeof(lf_top), "R:%s F:%s", r_buf, f_buf);
    snprintf(lf_br, sizeof(lf_br), "Stab:%d%%", _lf_stab_percent);
    props_to_fill->oled_bottom_props.graph_props.is_enabled = true;
    props_to_fill->oled_bottom_props.graph_props.pre_filter_data = _hf_filtered_buffer;
    props_to_fill->oled_bottom_props.graph_props.post_filter_data = _lf_filtered_buffer;
    props_to_fill->oled_bottom_props.graph_props.ghost_filter_data = _is_compare_mode_active ? _ghost_lf_filtered_buffer : nullptr;
    props_to_fill->oled_bottom_props.graph_props.top_left_label = lf_top;
    
    char cal_val_buf[20];
    if (isnan(_calibrated_value)) { snprintf(cal_val_buf, sizeof(cal_val_buf), "Val: ---"); }
    else if (_context->selectedFilter == &phFilter) { snprintf(cal_val_buf, sizeof(cal_val_buf), "pH: %.2f", _calibrated_value); }
    else { snprintf(cal_val_buf, sizeof(cal_val_buf), "EC: %.0f uS", _calibrated_value); }
    props_to_fill->oled_bottom_props.graph_props.bottom_left_label = cal_val_buf;
    
    props_to_fill->oled_bottom_props.graph_props.bottom_right_label = lf_br;
}