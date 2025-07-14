// src/presentation/screens/main_menu/diagnostics/N_Analysis/NoiseAnalysisScreen.h
// MODIFIED FILE
#pragma once

#include "presentation/screens/Screen.h"
#include <vector>
#include <string>

class NoiseAnalysisScreen : public Screen {
public:
    NoiseAnalysisScreen();
    // <<< MODIFIED: onEnter is no longer needed >>>
    void handleInput(const InputEvent& event) override;
    UIRenderProps getRenderProps() override;

private:
    enum class ViewState {
        SELECT_SENSOR,
        SHOW_ANALYSIS
    };
    ViewState _current_view;

    std::vector<std::string> _sensor_menu_items;
    int _selected_sensor_index;

    std::vector<float> _sample_data;
    std::vector<std::string> _view_options;
    int _selected_view_index;
    
    // <<< MODIFIED: _is_dirty flag removed >>>
};