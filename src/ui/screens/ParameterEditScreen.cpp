// File Path: /src/ui/screens/ParameterEditScreen.cpp
// NEW FILE

#include "ParameterEditScreen.h"
#include "pBiosContext.h"
#include <stdio.h> // For sprintf

ParameterEditScreen::ParameterEditScreen(PBiosContext* context) :
    _context(context),
    _param_index(-1)
{}

void ParameterEditScreen::setParameterToEdit(const std::string& name, int index) {
    _param_name = name;
    _param_index = index;
}

void ParameterEditScreen::handleInput(const InputEvent& event) {
    // --- FIX: The "Set" action is now triggered by the BOTTOM button ---
    if (event.type == InputEventType::BTN_DOWN_PRESS) {
        if (_stateManager) _stateManager->changeState(ScreenState::LIVE_FILTER_TUNING);
        return;
    }
    // The top button is now consistently "Back"
    if (event.type == InputEventType::BTN_BACK_PRESS) {
        // We can add logic here to cancel/revert the change if needed.
        // For now, it behaves the same as "Set".
        if (_stateManager) _stateManager->changeState(ScreenState::LIVE_FILTER_TUNING);
        return;
    }

    if (!_context || !_context->selectedFilter || _param_index < 0) return;
    
    PI_Filter* filter = (_param_index < 4) ? _context->selectedFilter->getFilter(0) : _context->selectedFilter->getFilter(1);
    if (!filter) return;
    
    int local_param_index = _param_index % 4;
    float step = (local_param_index == 0) ? 0.05f : (local_param_index == 3) ? 0.001f : 0.01f;
    float change = (event.type == InputEventType::ENCODER_INCREMENT) ? step : -step;

    switch (local_param_index) {
        case 0: filter->settleThreshold += change; if(filter->settleThreshold < 0) filter->settleThreshold = 0; break;
        case 1: filter->lockSmoothing += change; if(filter->lockSmoothing < 0) filter->lockSmoothing = 0; break;
        case 2: filter->trackResponse += change; if(filter->trackResponse < 0) filter->trackResponse = 0; break;
        case 3: filter->trackAssist += change; if(filter->trackAssist < 0) filter->trackAssist = 0; break;
    }
}

void ParameterEditScreen::getRenderProps(UIRenderProps* props_to_fill) {
    // --- Middle OLED: The Edit UI ---
    // Status area at the top
    props_to_fill->oled_middle_props.line1 = "Editing: " + _param_name;
    props_to_fill->oled_middle_props.line2 = "Value: " + getParamValueString();
    props_to_fill->oled_middle_props.line3 = "--------------------"; // Borderline

    // Instructions
    // These will appear below the borderline
    OledProps& mid_props = props_to_fill->oled_middle_props;
    mid_props.menu_props.is_enabled = false; // Disable menu drawing
    
    // We have to draw text manually since it's not a menu
    // This text appears at y=36, below the borderline
    // Note: Adafruit GFX doesn't support multiline strings, so we use separate prints
    // This will be handled by the UIManager drawing simple text lines if we set them up.
    // For now, let's keep it simple. We'll add text drawing logic later.
    
    // --- Button Prompts Updated ---
    props_to_fill->button_props.back_text = "Back";
    props_to_fill->button_props.enter_text = "";
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