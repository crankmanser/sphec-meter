// src/presentation/screens/main_menu/diagnostics/N_Analysis/NoiseAnalysisScreen.h
// MODIFIED FILE
#pragma once

#include "presentation/screens/Screen.h"
#include <vector>
#include <string>

class NoiseAnalysisScreen : public Screen {
public:
    NoiseAnalysisScreen();
    void handleInput(const InputEvent& event) override;
    UIRenderProps getRenderProps() override;

private:
    std::vector<float> _sample_data;
    
    // <<< ADDED: State for the new sub-menu on OLED #2 >>>
    std::vector<std::string> _view_options;
    int _selected_view_index;
};