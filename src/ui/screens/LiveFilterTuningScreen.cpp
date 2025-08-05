// File Path: /src/ui/screens/LiveFilterTuningScreen.cpp
// MODIFIED FILE

#include "LiveFilterTuningScreen.h"
#include "pBiosContext.h"
#include "AdcManager.h"
#include "CalibrationManager.h"
#include "TempManager.h"
#include <stdio.h>
#include <Arduino.h>

LiveFilterTuningScreen::LiveFilterTuningScreen(AdcManager* adcManager, PBiosContext* context, CalibrationManager* phCal, CalibrationManager* ecCal, TempManager* tempManager) :
    _adcManager(adcManager),
    _context(context),
    _phCalManager(phCal),
    _ecCalManager(ecCal),
    _tempManager(tempManager),
    _selected_index(0),
    _calibrated_value(NAN),
    _hf_f_std(0), _hf_r_std(0), _hf_stab_percent(0),
    _lf_f_std(0), _lf_r_std(0), _lf_stab_percent(0)
{
    for (int i = 0; i < GRAPH_DATA_POINTS; ++i) { _hf_raw_buffer[i] = 0.0; _hf_filtered_buffer[i] = 0.0; _lf_filtered_buffer[i] = 0.0; }
    _menu_item_names.push_back("HF Settle Thr");
    _menu_item_names.push_back("HF Lock Smooth");
    _menu_item_names.push_back("HF Track Resp");
    _menu_item_names.push_back("HF Track Assist");
    _menu_item_names.push_back("LF Settle Thr");
    _menu_item_names.push_back("LF Lock Smooth");
    _menu_item_names.push_back("LF Track Resp");
    _menu_item_names.push_back("LF Track Assist");
}

void LiveFilterTuningScreen::onEnter(StateManager* stateManager) { Screen::onEnter(stateManager); }

void LiveFilterTuningScreen::handleInput(const InputEvent& event) {
    if (event.type == InputEventType::BTN_BACK_PRESS) { if (_stateManager) _stateManager->changeState(ScreenState::FILTER_SELECTION); return; }
    if (event.type == InputEventType::ENCODER_INCREMENT) { if (_selected_index < _menu_item_names.size() - 1) _selected_index++; }
    else if (event.type == InputEventType::ENCODER_DECREMENT) { if (_selected_index > 0) _selected_index--; }
    else if (event.type == InputEventType::BTN_DOWN_PRESS) { if (_stateManager) _stateManager->changeState(ScreenState::PARAMETER_EDIT); }
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
}

void LiveFilterTuningScreen::setCalibratedValue(double value) { _calibrated_value = value; }

void LiveFilterTuningScreen::getRenderProps(UIRenderProps* props_to_fill) {
    static char hf_top[40], hf_bl[10], hf_br[20];
    static char lf_top[40], lf_bl[10], lf_br[20];
    char r_buf[10], f_buf[10];

    // --- HF Screen (Top OLED) ---
    dtostrf(_hf_r_std, 4, 3, r_buf);
    dtostrf(_hf_f_std, 4, 3, f_buf);
    snprintf(hf_top, sizeof(hf_top), "R:%s F:%s", r_buf, f_buf);
    snprintf(hf_bl, sizeof(hf_bl), "HF");
    snprintf(hf_br, sizeof(hf_br), "Stab:%d%%", _hf_stab_percent);
    
    // --- DEFINITIVE FIX: Use member-by-member assignment for the struct ---
    props_to_fill->oled_top_props.graph_props.is_enabled = true;
    props_to_fill->oled_top_props.graph_props.pre_filter_data = _hf_raw_buffer;
    props_to_fill->oled_top_props.graph_props.post_filter_data = _hf_filtered_buffer;
    props_to_fill->oled_top_props.graph_props.top_left_label = hf_top;
    props_to_fill->oled_top_props.graph_props.bottom_left_label = hf_bl;
    props_to_fill->oled_top_props.graph_props.bottom_right_label = hf_br;

    // --- LF Screen (Bottom OLED) ---
    dtostrf(_lf_r_std, 4, 3, r_buf);
    dtostrf(_lf_f_std, 4, 3, f_buf);
    snprintf(lf_top, sizeof(lf_top), "R:%s F:%s", r_buf, f_buf);
    snprintf(lf_bl, sizeof(lf_bl), "LF");
    snprintf(lf_br, sizeof(lf_br), "Stab:%d%%", _lf_stab_percent);

    // --- DEFINITIVE FIX: Use member-by-member assignment for the struct ---
    props_to_fill->oled_bottom_props.graph_props.is_enabled = true;
    props_to_fill->oled_bottom_props.graph_props.pre_filter_data = _hf_filtered_buffer;
    props_to_fill->oled_bottom_props.graph_props.post_filter_data = _lf_filtered_buffer;
    props_to_fill->oled_bottom_props.graph_props.top_left_label = lf_top;
    props_to_fill->oled_bottom_props.graph_props.bottom_left_label = lf_bl;
    props_to_fill->oled_bottom_props.graph_props.bottom_right_label = lf_br;

    // --- Menu Screen (Middle OLED) ---
    OledProps& mid_props = props_to_fill->oled_middle_props;
    char cal_val_buf[20];
    if (isnan(_calibrated_value)) {
        snprintf(cal_val_buf, sizeof(cal_val_buf), "pH: ---");
    } else if (_context->selectedFilter == &phFilter) {
        snprintf(cal_val_buf, sizeof(cal_val_buf), "pH: %.2f", _calibrated_value);
    } else {
        snprintf(cal_val_buf, sizeof(cal_val_buf), "EC: %.0f uS", _calibrated_value);
    }
    mid_props.line1 = cal_val_buf;
    mid_props.line2 = getSelectedParamValueString();
    mid_props.menu_props.is_enabled = true;
    mid_props.menu_props.items = _menu_item_names;
    mid_props.menu_props.selected_index = _selected_index;

    // --- Button Prompts ---
    props_to_fill->button_props.back_text = "Back";
    props_to_fill->button_props.down_text = "Edit";
}

const std::string& LiveFilterTuningScreen::getSelectedParamName() const { return _menu_item_names[_selected_index]; }
int LiveFilterTuningScreen::getSelectedParamIndex() const { return _selected_index; }
std::string LiveFilterTuningScreen::getSelectedParamValueString() {
    if (!_context || !_context->selectedFilter) return "N/A";
    PI_Filter* filter = (_selected_index < 4) ? _context->selectedFilter->getFilter(0) : _context->selectedFilter->getFilter(1);
    if (!filter) return "N/A";
    int param_index = _selected_index % 4;
    char buffer[10];
    double val = 0.0;
    switch (param_index) {
        case 0: val = filter->settleThreshold; dtostrf(val, 4, 2, buffer); break;
        case 1: val = filter->lockSmoothing; dtostrf(val, 4, 3, buffer); break;
        case 2: val = filter->trackResponse; dtostrf(val, 4, 3, buffer); break;
        case 3: val = filter->trackAssist; dtostrf(val, 4, 4, buffer); break;
    }
    return std::string(buffer);
}