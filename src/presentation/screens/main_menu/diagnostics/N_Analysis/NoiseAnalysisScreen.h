// src/presentation/screens/main_menu/diagnostics/N_Analysis/NoiseAnalysisScreen.h
// MODIFIED FILE
#pragma once

#include "presentation/screens/Screen.h"
#include "managers/diagnostics/NoiseAnalysisManager.h"
#include <vector>
#include <string>

// Forward-declare AppContext
struct AppContext;

class NoiseAnalysisScreen : public Screen {
public:
    NoiseAnalysisScreen(NoiseAnalysisManager* noiseAnalysisManager);
    void handleInput(const InputEvent& event) override;
    UIRenderProps getRenderProps() override;
    // <<< MODIFIED: Signature updated to match the base Screen class >>>
    void onEnter(StateManager* stateManager, AppContext* context) override;

private:
    enum class ViewState {
        SELECT_SENSOR,
        ANALYZING,
        SHOW_ANALYSIS
    };
    ViewState _current_view;

    std::vector<std::string> _sensor_menu_items;
    int _selected_sensor_index;

    // --- Member variables for analysis ---
    NoiseAnalysisManager* _noise_analysis_manager;
    NoiseAnalysisParams _analysis_params; 
    TaskHandle_t _analysis_task_handle;
    volatile bool _analysis_complete_flag;

    // --- Member variables for progress bar ---
    uint32_t _analysis_start_time_ms;
    uint32_t _estimated_duration_ms;

    // --- Member variables for results ---
    StatisticalResult _stat_results;
    FftResult _fft_results;
    PIFilter::Tuni_t _tuning_results;

    std::vector<std::string> _view_options;
    int _selected_view_index;
};