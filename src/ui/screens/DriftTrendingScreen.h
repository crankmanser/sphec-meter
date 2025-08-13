// File Path: /src/ui/screens/DriftTrendingScreen.h
// MODIFIED FILE

#ifndef DRIFT_TRENDING_SCREEN_H
#define DRIFT_TRENDING_SCREEN_H

#include "ui/StateManager.h"
#include <vector>
#include <string>

#define DRIFT_SAMPLE_COUNT 256
#define FFT_BIN_COUNT (DRIFT_SAMPLE_COUNT / 2)

struct PBiosContext;
class AdcManager;

/**
 * @class DriftTrendingScreen
 * @brief A pBIOS diagnostic screen for performing a long-duration FFT
 * analysis to identify low-frequency signal drift.
 * --- DEFINITIVE REFACTOR: This screen no longer manages hardware state. ---
 */
class DriftTrendingScreen : public Screen {
public:
    // --- DEFINITIVE REFACTOR: Constructor no longer needs the AdcManager ---
    DriftTrendingScreen(PBiosContext* context);
    void onEnter(StateManager* stateManager, int context = 0) override;

    // --- DEFINITIVE REFACTOR: onExit method is removed. ---
    // Probe power state is now handled centrally by the dataTask in main.cpp.

    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;

    void setAnalysisResults(const double* fft_magnitudes);
    void setSamplingProgress(int percent);
    void setAnalyzing();
    bool isSampling() const;
    int getSelectedDurationSec() const;

private:
    enum class TrendingState {
        SELECT_SOURCE,
        SELECT_DURATION,
        SAMPLING,
        ANALYZING,
        VIEW_RESULTS
    };

    void handleSelectSourceInput(const InputEvent& event);
    void handleSelectDurationInput(const InputEvent& event);
    void handleViewResultsInput(const InputEvent& event);
    
    void getSelectSourceRenderProps(UIRenderProps* props_to_fill);
    void getSelectDurationRenderProps(UIRenderProps* props_to_fill);
    void getSamplingRenderProps(UIRenderProps* props_to_fill);
    void getAnalyzingRenderProps(UIRenderProps* props_to_fill);
    void getViewResultsRenderProps(UIRenderProps* props_to_fill);

    // --- DEFINITIVE REFACTOR: AdcManager pointer is removed ---
    PBiosContext* _context;
    TrendingState _current_state;

    std::vector<std::string> _source_menu_items;
    int _selected_source_index;
    std::vector<std::string> _duration_menu_items;
    std::vector<int> _duration_values_sec;
    int _selected_duration_index;

    int _sampling_progress_percent;
    double _fft_results[FFT_BIN_COUNT];
};

#endif // DRIFT_TRENDING_SCREEN_H