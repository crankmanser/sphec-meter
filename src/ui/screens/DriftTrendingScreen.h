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

class DriftTrendingScreen : public Screen {
public:
    DriftTrendingScreen(PBiosContext* context, AdcManager* adcManager);
    void onEnter(StateManager* stateManager, int context = 0) override;
    
    /**
     * @brief --- NEW: Ensures the probe is deactivated on exit. ---
     * This is a fail-safe to prevent the probe from being left in an
     * active state if the user navigates away from this screen unexpectedly.
     */
    void onExit() override;

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

    PBiosContext* _context;
    AdcManager* _adcManager;
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