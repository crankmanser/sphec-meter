// File Path: /src/ui/screens/ParameterEditScreen.cpp
// MODIFIED FILE

#include "ParameterEditScreen.h"
#include "ui/screens/LiveFilterTuningScreen.h"
#include <stdio.h>

ParameterEditScreen::ParameterEditScreen(PBiosContext* context) :
    _context(context),
    _selected_index(0),
    _is_editing(false)
{
    _param_menu_items.push_back("HF Settle Thr");
    _param_menu_items.push_back("HF Lock Smooth");
    _param_menu_items.push_back("HF Track Resp");
    _param_menu_items.push_back("HF Track Assist");
    _param_menu_items.push_back("LF Settle Thr");
    _param_menu_items.push_back("LF Lock Smooth");
    _param_menu_items.push_back("LF Track Resp");
    _param_menu_items.push_back("LF Track Assist");
}

void ParameterEditScreen::onEnter(StateManager* stateManager) {
    Screen::onEnter(stateManager);
    _is_editing = false;
    _selected_index = 0;
    if (_context && _context->selectedFilter) {
        _hf_snapshot = *_context->selectedFilter->getFilter(0);
        _lf_snapshot = *_context->selectedFilter->getFilter(1);
    }
}

void ParameterEditScreen::handleInput(const InputEvent& event) {
    if (event.type == InputEventType::BTN_BACK_PRESS) {
        if (_is_editing) {
            _is_editing = false;
        } else {
            if (_context && _context->selectedFilter) {
                *_context->selectedFilter->getFilter(0) = _hf_snapshot;
                *_context->selectedFilter->getFilter(1) = _lf_snapshot;
            }
            if (_stateManager) _stateManager->changeState(ScreenState::LIVE_FILTER_TUNING);
        }
        return;
    }
    if (event.type == InputEventType::BTN_DOWN_PRESS) {
        if (_stateManager) _stateManager->changeState(ScreenState::LIVE_FILTER_TUNING);
        return;
    }
    if (event.type == InputEventType::BTN_ENTER_PRESS) {
        _is_editing = !_is_editing;
        return;
    }

    if (_is_editing) {
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
    } else {
        if (event.type == InputEventType::ENCODER_INCREMENT) { if (_selected_index < _param_menu_items.size() - 1) _selected_index++; }
        else if (event.type == InputEventType::ENCODER_DECREMENT) { if (_selected_index > 0) _selected_index--; }
    }
}

/**
 * @brief --- DEFINITIVE FIX: Rewritten to be self-contained. ---
 * This function no longer inherits render properties from the hub screen.
 * It now starts with a clean slate and calls the workbench's helper function
 * to draw the background, preventing any "ghost text" from bleeding through.
 */
void ParameterEditScreen::getRenderProps(UIRenderProps* props_to_fill) {
    // Start with a clean slate to prevent any state bleed-through.
    *props_to_fill = UIRenderProps();

    LiveFilterTuningScreen* workbench = static_cast<LiveFilterTuningScreen*>(_stateManager->getScreen(ScreenState::LIVE_FILTER_TUNING));
    if (!workbench) return;

    // 1. Get the background graph properties from the workbench's dedicated helper.
    workbench->getManualTuneRenderProps(props_to_fill);

    // 2. Overwrite the middle OLED and buttons with our own content.
    OledProps& mid_props = props_to_fill->oled_middle_props;
    mid_props.line2 = getSelectedParamValueString();
    mid_props.menu_props.is_enabled = true;
    mid_props.menu_props.items = _param_menu_items;
    mid_props.menu_props.selected_index = _selected_index;
    if (_is_editing) {
        mid_props.line3 = "> Editing <";
    }

    // 3. Set the correct button prompts for this screen.
    props_to_fill->button_props.back_text = "Cancel";
    props_to_fill->button_props.enter_text = _is_editing ? "OK" : "Edit";
    props_to_fill->button_props.down_text = "Set";
}

std::string ParameterEditScreen::getSelectedParamValueString() {
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