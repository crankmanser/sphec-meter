// File Path: /src/ui/screens/AutoTuningScreen.cpp
// NEW FILE

#include "AutoTuningScreen.h"

AutoTuningScreen::AutoTuningScreen() : _progress_percent(0) {}

// This screen is non-interactive, so input is ignored.
void AutoTuningScreen::handleInput(const InputEvent& event) {
    // No-op
}

void AutoTuningScreen::getRenderProps(UIRenderProps* props_to_fill) {
    props_to_fill->oled_top_props.line1 = "Guided Tuning";
    props_to_fill->oled_top_props.line2 = "Analyzing Signal...";

    props_to_fill->oled_middle_props.progress_bar_props.is_enabled = true;
    props_to_fill->oled_middle_props.progress_bar_props.label = "Calculating baseline...";
    props_to_fill->oled_middle_props.progress_bar_props.progress_percent = _progress_percent;

    props_to_fill->oled_bottom_props.line1 = "Please wait.";
}

void AutoTuningScreen::setProgress(int percent) {
    _progress_percent = percent;
    if (_progress_percent < 0) _progress_percent = 0;
    if (_progress_percent > 100) _progress_percent = 100;
}