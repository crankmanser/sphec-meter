// File Path: /src/ui/screens/LiveFilterTuningScreen.cpp
// MODIFIED FILE

#include "LiveFilterTuningScreen.h"
#include "pBiosContext.h"
#include <stdio.h> 

LiveFilterTuningScreen::LiveFilterTuningScreen(AdcManager* adcManager, PBiosContext* context) :
    _adcManager(adcManager),
    _context(context),
    _selected_index(0),
    _is_editing(false),
    _hf_f_std(0), _hf_r_std(0), _hf_stab_percent(0),
    _lf_f_std(0), _lf_r_std(0), _lf_stab_percent(0)
{
    for (int i = 0; i < GRAPH_DATA_POINTS; ++i) {
        _hf_raw_buffer[i] = 0.0;
        _hf_filtered_buffer[i] = 0.0;
        _lf_filtered_buffer[i] = 0.0;
    }

    _menu_item_names.push_back("HF Settle Threshold");
    _menu_item_names.push_back("HF Lock Smoothing");
    _menu_item_names.push_back("HF Track Response");
    _menu_item_names.push_back("HF Track Assist");
    _menu_item_names.push_back("LF Settle Threshold");
    _menu_item_names.push_back("LF Lock Smoothing");
    _menu_item_names.push_back("LF Track Response");
    _menu_item_names.push_back("LF Track Assist");
}

void LiveFilterTuningScreen::onEnter(StateManager* stateManager) {
    Screen::onEnter(stateManager);
    _is_editing = false;
}

void LiveFilterTuningScreen::handleInput(const InputEvent& event) {
    if (event.type == InputEventType::BTN_BACK_PRESS) {
        if (_stateManager) _stateManager->changeState(ScreenState::FILTER_SELECTION);
        return;
    }

    if (event.type == InputEventType::ENCODER_INCREMENT) {
        if (_selected_index < _menu_item_names.size() - 1) _selected_index++;
    } else if (event.type == InputEventType::ENCODER_DECREMENT) {
        if (_selected_index > 0) _selected_index--;
    } 
    else if (event.type == InputEventType::BTN_DOWN_PRESS) { 
        if (_stateManager) {
            _stateManager->changeState(ScreenState::PARAMETER_EDIT);
        }
    }
}

/**
 * @brief This method is now a data *harvester*. Instead of acquiring data,
 * it pulls the latest processed data from the filter's internal buffers, which
 * are being continuously updated by the pBiosDataTask. It then populates the
 * screen's local graph buffers with this historical data.
 */
void LiveFilterTuningScreen::update() {
    if (!_context || !_context->selectedFilter) return;

    PI_Filter* hfFilter = _context->selectedFilter->getFilter(0);
    PI_Filter* lfFilter = _context->selectedFilter->getFilter(1);

    // --- Data Harvesting ---
    if (hfFilter) {
        // Copy the historical data from the filter's internal buffers
        // into this screen's local buffers for graphing.
        hfFilter->getRawHistory(_hf_raw_buffer, GRAPH_DATA_POINTS);
        hfFilter->getFilteredHistory(_hf_filtered_buffer, GRAPH_DATA_POINTS);

        // Get the latest KPIs
        _hf_r_std = hfFilter->getRawStandardDeviation();
        _hf_f_std = hfFilter->getFilteredStandardDeviation();
        _hf_stab_percent = hfFilter->getStabilityPercentage();
    }
    
    if (lfFilter) {
        // The "raw" data for the LF graph is the filtered output of the HF stage.
        if (hfFilter) {
            hfFilter->getFilteredHistory(_lf_filtered_buffer, GRAPH_DATA_POINTS);
        }
        
        // Get the final, twice-filtered data for the graph's second line.
        lfFilter->getFilteredHistory(_lf_filtered_buffer, GRAPH_DATA_POINTS);

        // Get the latest KPIs for the LF stage
        _lf_r_std = hfFilter ? hfFilter->getFilteredStandardDeviation() : 0.0;
        _lf_f_std = lfFilter->getFilteredStandardDeviation();
        _lf_stab_percent = lfFilter->getStabilityPercentage();
    }
}


void LiveFilterTuningScreen::getRenderProps(UIRenderProps* props_to_fill) {
    char buffer[20];

    props_to_fill->oled_top_props.graph_props.is_enabled = true;
    props_to_fill->oled_top_props.graph_props.pre_filter_data = _hf_raw_buffer;
    props_to_fill->oled_top_props.graph_props.post_filter_data = _hf_filtered_buffer;
    
    props_to_fill->oled_top_props.graph_props.top_left_label = "HF Stage";
    snprintf(buffer, sizeof(buffer), "Stab: %d%%", _hf_stab_percent);
    props_to_fill->oled_top_props.graph_props.top_right_label = buffer;
    snprintf(buffer, sizeof(buffer), "R:%.2f", _hf_r_std);
    props_to_fill->oled_top_props.graph_props.bottom_left_label = buffer;
    snprintf(buffer, sizeof(buffer), "F:%.2f", _hf_f_std);
    props_to_fill->oled_top_props.graph_props.bottom_right_label = buffer;

    OledProps& mid_props = props_to_fill->oled_middle_props;
    std::string selected_value = getParamValueString(_selected_index);
    
    mid_props.line1 = getSelectedParamName();
    mid_props.line2 = selected_value;     
    mid_props.line3 = "";

    mid_props.menu_props.is_enabled = true;
    mid_props.menu_props.items = _menu_item_names;
    mid_props.menu_props.selected_index = _selected_index;

    props_to_fill->oled_bottom_props.graph_props.is_enabled = true;
    props_to_fill->oled_bottom_props.graph_props.pre_filter_data = _hf_filtered_buffer;
    props_to_fill->oled_bottom_props.graph_props.post_filter_data = _lf_filtered_buffer;

    props_to_fill->oled_bottom_props.graph_props.top_left_label = "LF Stage";
    snprintf(buffer, sizeof(buffer), "Stab: %d%%", _lf_stab_percent);
    props_to_fill->oled_bottom_props.graph_props.top_right_label = buffer;
    snprintf(buffer, sizeof(buffer), "R:%.2f", _lf_r_std);
    props_to_fill->oled_bottom_props.graph_props.bottom_left_label = buffer;
    snprintf(buffer, sizeof(buffer), "F:%.2f", _lf_f_std);
    props_to_fill->oled_bottom_props.graph_props.bottom_right_label = buffer;

    props_to_fill->button_props.back_text = "Back";
    props_to_fill->button_props.enter_text = ""; 
    props_to_fill->button_props.down_text = "Edit"; 
}

std::string LiveFilterTuningScreen::getParamValueString(int index) {
    if (!_context || !_context->selectedFilter) return "N/A";

    PI_Filter* filter = (index < 4) ? _context->selectedFilter->getFilter(0) : _context->selectedFilter->getFilter(1);
    if (!filter) return "N/A";

    int param_index = index % 4;
    char buffer[10];

    switch (param_index) {
        case 0: dtostrf(filter->settleThreshold, 4, 2, buffer); break;
        case 1: dtostrf(filter->lockSmoothing, 4, 3, buffer); break;
        case 2: dtostrf(filter->trackResponse, 4, 3, buffer); break;
        case 3: dtostrf(filter->trackAssist, 4, 4, buffer); break;
        default: return "N/A";
    }
    return std::string(buffer);
}

const std::string& LiveFilterTuningScreen::getSelectedParamName() const {
    return _menu_item_names[_selected_index];
}

int LiveFilterTuningScreen::getSelectedParamIndex() const {
    return _selected_index;
}