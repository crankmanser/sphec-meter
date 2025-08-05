// File Path: /src/ui/screens/ParameterEditScreen.cpp
// MODIFIED FILE

#include "ParameterEditScreen.h"
#include "pBiosContext.h"
#include <stdio.h>
#include <Arduino.h>

ParameterEditScreen::ParameterEditScreen(PBiosContext* context) :
    _context(context),
    _param_index(-1)
{}

void ParameterEditScreen::setParameterToEdit(const std::string& name, int index) {
    _param_name = name;
    _param_index = index;
}

void ParameterEditScreen::handleInput(const InputEvent& event) {
    if (event.type == InputEventType::BTN_BACK_PRESS || event.type == InputEventType::BTN_DOWN_PRESS) {
        if (_stateManager) _stateManager->changeState(ScreenState::LIVE_FILTER_TUNING);
        return;
    }

    if (!_context || !_context->selectedFilter || _param_index < 0) return;
    
    PI_Filter* filter = (_param_index < 4) ? _context->selectedFilter->getFilter(0) : _context->selectedFilter->getFilter(1);
    if (!filter) return;
    
    int local_param_index = _param_index % 4;
    float step = 0.01f; // Default step
    if(local_param_index == 3) step = 0.001f; // Finer step for track assist

    float change = (event.type == InputEventType::ENCODER_INCREMENT) ? step : -step;

    switch (local_param_index) {
        case 0: filter->settleThreshold += change; break;
        case 1: filter->lockSmoothing += change; break;
        case 2: filter->trackResponse += change; break;
        case 3: filter->trackAssist += change; break;
    }
}

/**
 * @brief --- STEP 3: Renders as a non-destructive overlay. ---
 * This function only modifies the properties for the middle OLED and buttons.
 * It leaves the top and bottom screens untouched, so the live graphs continue
 * to render without being frozen or overwritten.
 */
void ParameterEditScreen::getRenderProps(UIRenderProps* props_to_fill) {
    // --- Middle OLED: The editing UI ---
    OledProps& mid_props = props_to_fill->oled_middle_props;
    mid_props = OledProps(); // Clear it
    mid_props.line1 = "Editing: " + _param_name;
    mid_props.line2 = "New Value:";
    mid_props.line3 = getParamValueString(); // This will be inverted by the UIManager

    // --- Bottom OLED: Contextual Help ---
    // We only modify the text lines, leaving the graph props alone.
    OledProps& bottom_props = props_to_fill->oled_bottom_props;
    bottom_props.line1 = "Turn encoder to adjust.";
    bottom_props.line2 = "";
    bottom_props.line3 = "";

    // --- Button Prompts ---
    props_to_fill->button_props.back_text = "Cancel";
    props_to_fill->button_props.down_text = "Set";
}

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