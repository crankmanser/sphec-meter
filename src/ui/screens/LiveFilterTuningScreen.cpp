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

LiveFilterTuningScreen::LiveFilterTuningScreen(AdcManager* adcManager, PBiosContext* context, CalibrationManager* phCal, CalibrationManager* ecCal, TempManager* tempManager) :
    _adcManager(adcManager),
    _context(context),
    _phCalManager(phCal),
    _ecCalManager(ecCal),
    _tempManager(tempManager),
    _current_state(WorkbenchState::HUB_MENU),
    _selected_index(0),
    _is_compare_mode_active(false),
    _calibrated_value(NAN),
    _hf_f_std(0), _hf_r_std(0), _hf_stab_percent(0),
    _lf_f_std(0), _lf_r_std(0), _lf_stab_percent(0)
{
    for (int i = 0; i < GRAPH_DATA_POINTS; ++i) { 
        _hf_raw_buffer[i] = 0.0; 
        _hf_filtered_buffer[i] = 0.0; 
        _lf_filtered_buffer[i] = 0.0; 
        _ghost_lf_filtered_buffer[i] = 0.0;
    }

    _hub_menu_items.push_back("Auto Tune");
    _hub_menu_items.push_back("Manual Tune");
    _hub_menu_items.push_back("Save Current Tune");
    _hub_menu_items.push_back("Restore Saved Tune");
    _hub_menu_items.push_back("Exit");

    _hub_menu_descriptions.push_back("Run the automated tuning wizard.");
    _hub_menu_descriptions.push_back("Manually adjust all filter parameters.");
    _hub_menu_descriptions.push_back("Save the current settings to the SD card.");
    _hub_menu_descriptions.push_back("Load the last saved settings.");
    _hub_menu_descriptions.push_back("Return to the filter selection screen.");

    _param_menu_items.push_back("HF Settle Thr");
    _param_menu_items.push_back("HF Lock Smooth");
    _param_menu_items.push_back("HF Track Resp");
    _param_menu_items.push_back("HF Track Assist");
    _param_menu_items.push_back("LF Settle Thr");
    _param_menu_items.push_back("LF Lock Smooth");
    _param_menu_items.push_back("LF Track Resp");
    _param_menu_items.push_back("LF Track Assist");
}

void LiveFilterTuningScreen::onEnter(StateManager* stateManager) {
    Screen::onEnter(stateManager);
    _current_state = WorkbenchState::HUB_MENU;
    _selected_index = 0;
    _is_compare_mode_active = false;
}

void LiveFilterTuningScreen::handleInput(const InputEvent& event) {
    switch (_current_state) {
        case WorkbenchState::HUB_MENU: handleHubMenuInput(event); break;
        case WorkbenchState::MANUAL_TUNE: case WorkbenchState::MANUAL_TUNE_EDITING: handleManualTuneInput(event); break;
    }
}

void LiveFilterTuningScreen::getRenderProps(UIRenderProps* props_to_fill) {
    *props_to_fill = UIRenderProps();
    switch (_current_state) {
        case WorkbenchState::HUB_MENU: getHubMenuRenderProps(props_to_fill); break;
        case WorkbenchState::MANUAL_TUNE: case WorkbenchState::MANUAL_TUNE_EDITING: getManualTuneRenderProps(props_to_fill); break;
    }
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
            if (_context && _context->selectedFilter) {
                _manual_tune_snapshot.hf_params = *_context->selectedFilter->getFilter(0);
                _manual_tune_snapshot.lf_params = *_context->selectedFilter->getFilter(1);
            }
            _current_state = WorkbenchState::MANUAL_TUNE;
            _selected_index = 0;
        }
        else if (selected_item == "Save Current Tune") {
            if (_context && _context->selectedFilter) {
                configManager.saveFilterSettings(*_context->selectedFilter, _context->selectedFilterName.c_str(), true);
                configManager.saveFilterSettings(*_context->selectedFilter, _context->selectedFilterName.c_str(), false);
            }
        }
        else if (selected_item == "Restore Saved Tune") {
            if (_context && _context->selectedFilter) {
                configManager.loadFilterSettings(*_context->selectedFilter, _context->selectedFilterName.c_str(), true);
            }
        }
        else if (selected_item == "Exit") { if (_stateManager) _stateManager->changeState(ScreenState::FILTER_SELECTION); }
    }
}

void LiveFilterTuningScreen::getHubMenuRenderProps(UIRenderProps* props_to_fill) {
    if (_selected_index < _hub_menu_descriptions.size()) {
        props_to_fill->oled_top_props.line1 = _hub_menu_descriptions[_selected_index];
    }
    OledProps& mid_props = props_to_fill->oled_middle_props;
    mid_props.menu_props.is_enabled = true;
    mid_props.menu_props.items = _hub_menu_items;
    mid_props.menu_props.selected_index = _selected_index;
    props_to_fill->oled_bottom_props.line1 = "pBios > Tuning Workbench";
    char cal_val_buf[20];
    if (isnan(_calibrated_value)) { snprintf(cal_val_buf, sizeof(cal_val_buf), "Val: ---"); }
    else if (_context->selectedFilter == &phFilter) { snprintf(cal_val_buf, sizeof(cal_val_buf), "pH: %.2f", _calibrated_value); }
    else { snprintf(cal_val_buf, sizeof(cal_val_buf), "EC: %.0f uS", _calibrated_value); }
    props_to_fill->oled_bottom_props.line2 = cal_val_buf;
    props_to_fill->button_props.down_text = "Select";
}

void LiveFilterTuningScreen::handleManualTuneInput(const InputEvent& event) {
    if (event.type == InputEventType::BTN_BACK_PRESS) {
        if (_current_state == WorkbenchState::MANUAL_TUNE_EDITING) {
            _current_state = WorkbenchState::MANUAL_TUNE;
        } else {
            *_context->selectedFilter->getFilter(0) = _manual_tune_snapshot.hf_params;
            *_context->selectedFilter->getFilter(1) = _manual_tune_snapshot.lf_params;
            _current_state = WorkbenchState::HUB_MENU;
            _selected_index = 0;
        }
        return;
    }
    if (event.type == InputEventType::BTN_DOWN_PRESS) {
        _current_state = WorkbenchState::HUB_MENU;
        _selected_index = 0;
        return;
    }
    if (event.type == InputEventType::BTN_ENTER_PRESS) {
        if (_current_state == WorkbenchState::MANUAL_TUNE_EDITING) {
            _current_state = WorkbenchState::MANUAL_TUNE;
        } else {
            _current_state = WorkbenchState::MANUAL_TUNE_EDITING;
        }
        return;
    }

    if (_current_state == WorkbenchState::MANUAL_TUNE) {
        if (event.type == InputEventType::ENCODER_INCREMENT) { if (_selected_index < _param_menu_items.size() - 1) _selected_index++; }
        else if (event.type == InputEventType::ENCODER_DECREMENT) { if (_selected_index > 0) _selected_index--; }
    }
    else if (_current_state == WorkbenchState::MANUAL_TUNE_EDITING) {
        PI_Filter* filter = (_selected_index < 4) ? _context->selectedFilter->getFilter(0) : _context->selectedFilter->getFilter(1);
        if (!filter) return;
        int local_param_index = _selected_index % 4;
        double step = (local_param_index == 3) ? 0.001 * event.value : 0.01 * event.value;
        double change = (event.type == InputEventType::ENCODER_INCREMENT) ? step : -step;
        switch (local_param_index) {
            case 0: filter->settleThreshold += change; break;
            case 1: filter->lockSmoothing += change; break;
            case 2: filter->trackResponse += change; break;
            case 3: filter->trackAssist += change; break;
        }
    }
}

void LiveFilterTuningScreen::getManualTuneRenderProps(UIRenderProps* props_to_fill) {
    static char hf_top[40], hf_bl[10], hf_br[20], lf_top[40], lf_bl[10], lf_br[20], r_buf[10], f_buf[10];
    
    // --- DEFINITIVE FIX: Use member-by-member assignment to fix compiler error ---
    dtostrf(_hf_r_std, 4, 3, r_buf); dtostrf(_hf_f_std, 4, 3, f_buf);
    snprintf(hf_top, sizeof(hf_top), "R:%s F:%s", r_buf, f_buf);
    snprintf(hf_bl, sizeof(hf_bl), "HF"); snprintf(hf_br, sizeof(hf_br), "Stab:%d%%", _hf_stab_percent);
    props_to_fill->oled_top_props.graph_props.is_enabled = true;
    props_to_fill->oled_top_props.graph_props.pre_filter_data = _hf_raw_buffer;
    props_to_fill->oled_top_props.graph_props.post_filter_data = _hf_filtered_buffer;
    props_to_fill->oled_top_props.graph_props.top_left_label = hf_top;
    props_to_fill->oled_top_props.graph_props.bottom_left_label = hf_bl;
    props_to_fill->oled_top_props.graph_props.bottom_right_label = hf_br;
    
    dtostrf(_lf_r_std, 4, 3, f_buf); dtostrf(_lf_f_std, 4, 3, r_buf);
    snprintf(lf_top, sizeof(lf_top), "R:%s F:%s", f_buf, r_buf);
    snprintf(lf_bl, sizeof(lf_bl), "LF"); snprintf(lf_br, sizeof(lf_br), "Stab:%d%%", _lf_stab_percent);
    props_to_fill->oled_bottom_props.graph_props.is_enabled = true;
    props_to_fill->oled_bottom_props.graph_props.pre_filter_data = _hf_filtered_buffer;
    props_to_fill->oled_bottom_props.graph_props.post_filter_data = _lf_filtered_buffer;
    props_to_fill->oled_bottom_props.graph_props.ghost_filter_data = _is_compare_mode_active ? _ghost_lf_filtered_buffer : nullptr;
    props_to_fill->oled_bottom_props.graph_props.top_left_label = lf_top;
    props_to_fill->oled_bottom_props.graph_props.bottom_left_label = lf_bl;
    props_to_fill->oled_bottom_props.graph_props.bottom_right_label = lf_br;
    
    OledProps& mid_props = props_to_fill->oled_middle_props;
    mid_props.line1 = getSelectedParamValueString();
    mid_props.menu_props.is_enabled = true;
    mid_props.menu_props.items = _param_menu_items;
    mid_props.menu_props.selected_index = _selected_index;
    if (_current_state == WorkbenchState::MANUAL_TUNE_EDITING) {
        mid_props.line2 = "> Editing <";
    }

    props_to_fill->button_props.back_text = "Cancel";
    props_to_fill->button_props.enter_text = _current_state == WorkbenchState::MANUAL_TUNE_EDITING ? "OK" : "Edit";
    props_to_fill->button_props.down_text = "Set";
}

void LiveFilterTuningScreen::runCompareModeSimulation() {
    if (!_context || !_context->selectedFilter) return;
    FilterManager* tempFilter = new FilterManager();
    if (!tempFilter) return;
    if (configManager.loadFilterSettings(*tempFilter, _context->selectedFilterName.c_str(), true)) {
        PI_Filter* hfSim = tempFilter->getFilter(0);
        PI_Filter* lfSim = tempFilter->getFilter(1);
        if (hfSim && lfSim) {
            for (int i = 0; i < GRAPH_DATA_POINTS; ++i) {
                double hf_out = hfSim->process(_hf_raw_buffer[i]);
                _ghost_lf_filtered_buffer[i] = lfSim->process(hf_out);
            }
        }
    }
    delete tempFilter;
}

std::string LiveFilterTuningScreen::getSelectedParamValueString() {
    if (!_context || !_context->selectedFilter) return "N/A";
    PI_Filter* filter = (_selected_index < 4) ? _context->selectedFilter->getFilter(0) : _context->selectedFilter->getFilter(1);
    if (!filter) return "N/A";
    int param_index = _selected_index % 4;
    char buffer[20];
    double val = 0.0;
    switch (param_index) {
        case 0: val = filter->settleThreshold; dtostrf(val, 4, 3, buffer); break;
        case 1: val = filter->lockSmoothing; dtostrf(val, 4, 3, buffer); break;
        case 2: val = filter->trackResponse; dtostrf(val, 4, 3, buffer); break;
        case 3: val = filter->trackAssist; dtostrf(val, 4, 4, buffer); break;
    }
    return std::string("Value: ") + buffer;
}