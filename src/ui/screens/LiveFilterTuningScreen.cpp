// File Path: /src/ui/screens/LiveFilterTuningScreen.cpp
// MODIFIED FILE

#include "LiveFilterTuningScreen.h"
#include "pBiosContext.h" // <<< FIX: Include the new context header

LiveFilterTuningScreen::LiveFilterTuningScreen(AdcManager* adcManager, PBiosContext* context) :
    _adcManager(adcManager),
    _context(context),
    _selected_index(0)
{
    _menu_items.push_back("HF Settle Threshold");
    _menu_items.push_back("HF Lock Smoothing");
    _menu_items.push_back("HF Track Response");
    _menu_items.push_back("HF Track Assist");
    _menu_items.push_back("LF Settle Threshold");
    _menu_items.push_back("LF Lock Smoothing");
    _menu_items.push_back("LF Track Response");
    _menu_items.push_back("LF Track Assist");
}

void LiveFilterTuningScreen::handleInput(const InputEvent& event) {
    if (event.type == InputEventType::BTN_BACK_PRESS) {
        if (_stateManager) _stateManager->changeState(ScreenState::PBIOS_MENU);
    }
    else if (event.type == InputEventType::ENCODER_INCREMENT) {
        if (_selected_index < _menu_items.size() - 1) {
            _selected_index++;
        }
    }
    else if (event.type == InputEventType::ENCODER_DECREMENT) {
        if (_selected_index > 0) {
            _selected_index--;
        }
    }
}

void LiveFilterTuningScreen::update() {
    if (!_adcManager || !_context || !_context->selectedFilter) return;

    FilterManager* filterToTune = _context->selectedFilter;
    uint8_t adcIndex = _context->selectedAdcIndex;
    uint8_t adcInput = _context->selectedAdcInput;

    PI_Filter* hfFilter = filterToTune->getFilter(0);
    PI_Filter* lfFilter = filterToTune->getFilter(1);

    for (int i = 0; i < GRAPH_DATA_POINTS; ++i) {
        double raw_voltage = _adcManager->getVoltage(adcIndex, adcInput);
        _hf_raw_buffer[i] = raw_voltage;

        if (hfFilter) {
            _hf_filtered_buffer[i] = hfFilter->process(raw_voltage);
        } else {
            _hf_filtered_buffer[i] = raw_voltage;
        }

        if (lfFilter) {
            _lf_filtered_buffer[i] = lfFilter->process(_hf_filtered_buffer[i]);
        } else {
            _lf_filtered_buffer[i] = _hf_filtered_buffer[i];
        }
        delayMicroseconds(1000); 
    }
    // ... (rest of update is unchanged) ...
}

void LiveFilterTuningScreen::getRenderProps(UIRenderProps* props_to_fill) {
    // --- Top OLED: HF Graph ---
    props_to_fill->oled_top_props.graph_props.is_enabled = true;
    props_to_fill->oled_top_props.graph_props.pre_filter_data = _hf_raw_buffer;
    props_to_fill->oled_top_props.graph_props.post_filter_data = _hf_filtered_buffer;
    // TODO: Populate KPI labels for the HF graph
    props_to_fill->oled_top_props.graph_props.top_left_label = "HF Stage";

    // --- Middle OLED: The Menu ---
    props_to_fill->oled_middle_props.menu_props.is_enabled = true;
    props_to_fill->oled_middle_props.menu_props.items = _menu_items;
    props_to_fill->oled_middle_props.menu_props.selected_index = _selected_index;

    // --- Bottom OLED: LF Graph ---
    props_to_fill->oled_bottom_props.graph_props.is_enabled = true;
    // The "raw" data for the LF graph is the output of the HF filter
    props_to_fill->oled_bottom_props.graph_props.pre_filter_data = _hf_filtered_buffer;
    props_to_fill->oled_bottom_props.graph_props.post_filter_data = _lf_filtered_buffer;
    // TODO: Populate KPI labels for the LF graph
    props_to_fill->oled_bottom_props.graph_props.top_left_label = "LF Stage";

    // --- Button Prompts ---
    props_to_fill->button_props.back_text = "Back";
    props_to_fill->button_props.enter_text = "Edit";
}