// File Path: /src/ui/screens/AutoTuningScreen.cpp
// MODIFIED FILE

#include "AutoTuningScreen.h"

AutoTuningScreen::AutoTuningScreen() : 
    _progress_percent(0),
    _progress_label("Initializing...") // Set a default starting label
{}

// This screen is non-interactive while the process is running.
void AutoTuningScreen::handleInput(const InputEvent& event) {
    // No-op for now
}

void AutoTuningScreen::getRenderProps(UIRenderProps* props_to_fill) {
    props_to_fill->oled_top_props.line1 = "Guided Tuning";
    props_to_fill->oled_top_props.line2 = "Please wait.";

    props_to_fill->oled_middle_props.progress_bar_props.is_enabled = true;
    props_to_fill->oled_middle_props.progress_bar_props.label = _progress_label;
    props_to_fill->oled_middle_props.progress_bar_props.progress_percent = _progress_percent;

    props_to_fill->oled_bottom_props.line1 = "This may take a few moments...";
}

/**
 * @brief --- DEFINITIVE FIX: Implements the updated function with two parameters. ---
 */
void AutoTuningScreen::setProgress(int percent, const std::string& label) {
    _progress_percent = percent;
    _progress_label = label;
    if (_progress_percent < 0) _progress_percent = 0;
    if (_progress_percent > 100) _progress_percent = 100;
}