// File Path: /src/ui/screens/ParameterEditScreen.cpp
// MODIFIED FILE

#include "ParameterEditScreen.h"
#include "pBiosContext.h"
#include <stdio.h>
#include <Arduino.h>

// --- NEW: A temporary struct to hold parameter values ---
struct ParamSnapshot {
    double settleThreshold;
    double lockSmoothing;
    double trackResponse;
    double trackAssist;
};

// --- NEW: Two snapshots to store pre-edit values ---
ParamSnapshot hf_snapshot;
ParamSnapshot lf_snapshot;


ParameterEditScreen::ParameterEditScreen(PBiosContext* context) :
    _context(context),
    _param_index(-1)
{}

void ParameterEditScreen::setParameterToEdit(const std::string& name, int index) {
    _param_name = name;
    _param_index = index;

    // --- NEW: Take a snapshot of parameters when entering the screen ---
    if (_context && _context->selectedFilter) {
        PI_Filter* hf = _context->selectedFilter->getFilter(0);
        PI_Filter* lf = _context->selectedFilter->getFilter(1);
        if (hf) {
            hf_snapshot = {hf->settleThreshold, hf->lockSmoothing, hf->trackResponse, hf->trackAssist};
        }
        if (lf) {
            lf_snapshot = {lf->settleThreshold, lf->lockSmoothing, lf->trackResponse, lf->trackAssist};
        }
    }
}

void ParameterEditScreen::handleInput(const InputEvent& event) {
    // --- MODIFIED: Handle "Cancel" button press ---
    if (event.type == InputEventType::BTN_BACK_PRESS) {
        // Restore the snapshot if the user cancels
        if (_context && _context->selectedFilter) {
            PI_Filter* hf = _context->selectedFilter->getFilter(0);
            PI_Filter* lf = _context->selectedFilter->getFilter(1);
            if (hf) {
                hf->settleThreshold = hf_snapshot.settleThreshold;
                hf->lockSmoothing = hf_snapshot.lockSmoothing;
                hf->trackResponse = hf_snapshot.trackResponse;
                hf->trackAssist = hf_snapshot.trackAssist;
            }
            if (lf) {
                lf->settleThreshold = lf_snapshot.settleThreshold;
                lf->lockSmoothing = lf_snapshot.lockSmoothing;
                lf->trackResponse = lf_snapshot.trackResponse;
                lf->trackAssist = lf_snapshot.trackAssist;
            }
        }
        if (_stateManager) _stateManager->changeState(ScreenState::LIVE_FILTER_TUNING);
        return;
    }

    // --- MODIFIED: Handle "Set" button press ---
    // The actual saving is handled in main.cpp, this just transitions state.
    if (event.type == InputEventType::BTN_DOWN_PRESS) {
        if (_stateManager) _stateManager->changeState(ScreenState::LIVE_FILTER_TUNING);
        return;
    }

    if (!_context || !_context->selectedFilter || _param_index < 0) return;
    
    PI_Filter* filter = (_param_index < 4) ? _context->selectedFilter->getFilter(0) : _context->selectedFilter->getFilter(1);
    if (!filter) return;
    
    int local_param_index = _param_index % 4;
    double step = 0.01;
    if(local_param_index == 0) step = 0.01;
    if(local_param_index == 1) step = 0.01;
    if(local_param_index == 2) step = 0.01;
    if(local_param_index == 3) step = 0.001;

    double change = (event.type == InputEventType::ENCODER_INCREMENT) ? step : -step;

    switch (local_param_index) {
        case 0: filter->settleThreshold += change; break;
        case 1: filter->lockSmoothing += change; break;
        case 2: filter->trackResponse += change; break;
        case 3: filter->trackAssist += change; break;
    }
}

void ParameterEditScreen::getRenderProps(UIRenderProps* props_to_fill) {
    OledProps& mid_props = props_to_fill->oled_middle_props;
    mid_props = OledProps();
    mid_props.line1 = "Editing: " + _param_name;
    mid_props.line2 = "New Value:";
    
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "> %s <", getParamValueString().c_str());
    mid_props.line3 = buffer;

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
        case 0: val = filter->settleThreshold; dtostrf(val, 4, 3, buffer); break;
        case 1: val = filter->lockSmoothing; dtostrf(val, 4, 3, buffer); break;
        case 2: val = filter->trackResponse; dtostrf(val, 4, 3, buffer); break;
        case 3: val = filter->trackAssist; dtostrf(val, 4, 4, buffer); break;
    }
    return std::string(buffer);
}