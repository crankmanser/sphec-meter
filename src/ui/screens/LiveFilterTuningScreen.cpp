// File Path: /src/ui/screens/LiveFilterTuningScreen.cpp
// MODIFIED FILE

#include "LiveFilterTuningScreen.h"
#include "pBiosContext.h"
#include <stdio.h>

LiveFilterTuningScreen::LiveFilterTuningScreen(AdcManager* adcManager, PBiosContext* context) :
    _adcManager(adcManager),
    _context(context),
    _selected_index(0),
    _hf_f_std(0), _hf_r_std(0), _hf_stab_percent(0),
    _lf_f_std(0), _lf_r_std(0), _lf_stab_percent(0)
{
    // Store just the base names of the parameters
    _menu_item_names.push_back("HF Settle Threshold");
    _menu_item_names.push_back("HF Lock Smoothing");
    _menu_item_names.push_back("HF Track Response");
    _menu_item_names.push_back("HF Track Assist");
    _menu_item_names.push_back("LF Settle Threshold");
    _menu_item_names.push_back("LF Lock Smoothing");
    _menu_item_names.push_back("LF Track Response");
    _menu_item_names.push_back("LF Track Assist");
}

/**
 * @brief Called when the screen becomes active.
 * Ensures we reset the editing state every time we enter the screen.
 */
void LiveFilterTuningScreen::onEnter(StateManager* stateManager) {
    Screen::onEnter(stateManager);
    _is_editing = false;
}

/**
 * @brief Handles user input, now with logic for both navigation and editing.
 */
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
    // --- FIX: The "Edit" action is now triggered by the BOTTOM button ---
    else if (event.type == InputEventType::BTN_DOWN_PRESS) { 
        if (_stateManager) {
            // The logic to transition to the edit screen remains the same
            _stateManager->changeState(ScreenState::PARAMETER_EDIT);
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

/**
 * @brief Populates the render properties, now with dynamic menu text and button prompts.
 */
void LiveFilterTuningScreen::getRenderProps(UIRenderProps* props_to_fill) {
    char buffer[20];


    // --- Top OLED: HF Graph ---
    props_to_fill->oled_top_props.graph_props.is_enabled = true;
    props_to_fill->oled_top_props.graph_props.pre_filter_data = _hf_raw_buffer;
    props_to_fill->oled_top_props.graph_props.post_filter_data = _hf_filtered_buffer;
    // TODO: Populate KPI labels for the HF graph
    props_to_fill->oled_top_props.graph_props.top_left_label = "HF Stage";

    // --- Middle OLED: New Layout ---
    OledProps& mid_props = props_to_fill->oled_middle_props;

    // We will now handle the drawing of the status area manually within the UIManager
    // For now, let's just pass the value. The screen itself will be responsible for drawing.
    // This is a temporary step until we create a dedicated block for this.
    std::string selected_value = getParamValueString(_selected_index);
    
    // Clear the simple text lines, as we'll handle drawing ourselves
    mid_props.line1 = "";
    mid_props.line2 = "";
    mid_props.line3 = "";

    // Menu (will be drawn lower on the screen)
    mid_props.menu_props.is_enabled = true;
    mid_props.menu_props.items = _menu_item_names;
    mid_props.menu_props.selected_index = _selected_index;
    
    // --- We will add custom drawing logic here in a moment ---
    // For now, let's pass the value to a temporary unused field
    // so we can access it in the UIManager.
    mid_props.line1 = getSelectedParamName(); // Pass name for context
    mid_props.line2 = selected_value;      // Pass value to be drawn

    // --- Bottom OLED: LF Graph ---
    props_to_fill->oled_bottom_props.graph_props.is_enabled = true;
    // The "raw" data for the LF graph is the output of the HF filter
    props_to_fill->oled_bottom_props.graph_props.pre_filter_data = _hf_filtered_buffer;
    props_to_fill->oled_bottom_props.graph_props.post_filter_data = _lf_filtered_buffer;
    // TODO: Populate KPI labels for the LF graph
    props_to_fill->oled_bottom_props.graph_props.top_left_label = "LF Stage";

    // --- Button Prompts ---
    props_to_fill->button_props.back_text = "Back";
    // --- FIX: Middle button now has no prompt on this screen ---
    props_to_fill->button_props.enter_text = ""; 
    // --- FIX: Bottom button is now the "Edit" button ---
    props_to_fill->button_props.down_text = "Edit"; 
}

/**
 * @brief Helper function to get the formatted string for a parameter's current value.
 */
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

// --- Implementations for the public getters ---
const std::string& LiveFilterTuningScreen::getSelectedParamName() const {
    return _menu_item_names[_selected_index];
}

int LiveFilterTuningScreen::getSelectedParamIndex() const {
    return _selected_index;
}