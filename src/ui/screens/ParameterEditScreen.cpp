// File Path: /src/ui/screens/ParameterEditScreen.cpp
// MODIFIED FILE

#include "ParameterEditScreen.h"
#include "pBiosContext.h"
#include <stdio.h>
#include <Arduino.h> // Required for dtostrf

// ... (constructor, setParameterToEdit, and handleInput are unchanged) ...
ParameterEditScreen::ParameterEditScreen(PBiosContext* context) :
    _context(context),
    _param_index(-1)
{}
void ParameterEditScreen::setParameterToEdit(const std::string& name, int index) {
    _param_name = name;
    _param_index = index;
}
void ParameterEditScreen::handleInput(const InputEvent& event) {
    if (event.type == InputEventType::BTN_DOWN_PRESS) {
        if (_stateManager) _stateManager->changeState(ScreenState::LIVE_FILTER_TUNING);
        return;
    }
    if (event.type == InputEventType::BTN_BACK_PRESS) {
        if (_stateManager) _stateManager->changeState(ScreenState::LIVE_FILTER_TUNING);
        return;
    }
    if (!_context || !_context->selectedFilter || _param_index < 0) return;
    PI_Filter* filter = (_param_index < 4) ? _context->selectedFilter->getFilter(0) : _context->selectedFilter->getFilter(1);
    if (!filter) return;
    int local_param_index = _param_index % 4;
    float step = 0.0f;
    switch(local_param_index) {
        case 0: step = 0.05f; break;
        case 1: step = 0.01f; break;
        case 2: step = 0.01f; break;
        case 3: step = 0.001f; break;
    }
    float change = (event.type == InputEventType::ENCODER_INCREMENT) ? step : -step;
    switch (local_param_index) {
        case 0: filter->settleThreshold += change; if(filter->settleThreshold < 0) filter->settleThreshold = 0; break;
        case 1: filter->lockSmoothing += change; if(filter->lockSmoothing < 0) filter->lockSmoothing = 0; break;
        case 2: filter->trackResponse += change; if(filter->trackResponse < 0) filter->trackResponse = 0; break;
        case 3: filter->trackAssist += change; if(filter->trackAssist < 0) filter->trackAssist = 0; break;
    }
}


/**
 * @brief --- CRITICAL FIX: Redesigned the edit screen to be a true, non-overlapping overlay. ---
 */
void ParameterEditScreen::getRenderProps(UIRenderProps* props_to_fill) {
    // Intentionally do NOT modify the top or bottom props, only the middle and buttons.
    OledProps& mid_props = props_to_fill->oled_middle_props;
    mid_props = OledProps(); // Clear just the middle props

    // Line 1: The name of the parameter being edited.
    mid_props.line1 = "Editing:";
    mid_props.line2 = _param_name;
    
    // Line 3: The current value. The UIManager will draw this with an inverse box.
    mid_props.line3 = getParamValueString();

    // Use the now-empty bottom screen for contextual help.
    props_to_fill->oled_bottom_props.line1 = "Use encoder to adjust value.";
    props_to_fill->oled_bottom_props.line2 = "";
    props_to_fill->oled_bottom_props.line3 = "";
    props_to_fill->oled_bottom_props.graph_props.is_enabled = false;
    
    // Update button prompts for the editing context
    props_to_fill->button_props.back_text = "Cancel";
    props_to_fill->button_props.enter_text = "";
    props_to_fill->button_props.down_text = "Set";
}






// ... (getParamValueString is unchanged) ...
std::string ParameterEditScreen::getParamValueString() {
    if (!_context || !_context->selectedFilter || _param_index < 0) return "N/A";
    PI_Filter* filter = (_param_index < 4) ? _context->selectedFilter->getFilter(0) : _context->selectedFilter->getFilter(1);
    
    if (!filter) return "N/A";
    
    int param_index = _param_index % 4;
    char buffer[10];
    double val = 0;
    
    switch (param_index) {
        case 0: val = filter->settleThreshold; break;
        case 1: val = filter->lockSmoothing; break;
        case 2: val = filter->trackResponse; break;
        case 3: val = filter->trackAssist; break;
    }
    
    dtostrf(val, 4, 4, buffer);
    return std::string(buffer);
}