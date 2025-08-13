// File Path: /src/ui/screens/NoiseAnalysisScreen.h
// MODIFIED FILE

#ifndef NOISE_ANALYSIS_SCREEN_H
#define NOISE_ANALYSIS_SCREEN_H

#include "ui/StateManager.h"
#include <vector>
#include <string>

#define ANALYSIS_SAMPLE_COUNT 512

// Forward declarations to avoid circular dependencies
struct PBiosContext;
class AdcManager;

/**
 * @class NoiseAnalysisScreen
 * @brief A pBIOS diagnostic screen for performing a high-speed statistical
 * analysis of a selected signal source.
 * --- DEFINITIVE REFACTOR: This screen no longer manages hardware state. ---
 */
class NoiseAnalysisScreen : public Screen {
public:
    // --- DEFINITIVE REFACTOR: Constructor no longer needs the AdcManager ---
    NoiseAnalysisScreen(PBiosContext* context);
    void onEnter(StateManager* stateManager, int context = 0) override;

    // --- DEFINITIVE REFACTOR: onExit method is removed. ---
    // Probe power state is now handled centrally by the dataTask in main.cpp.

    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;

    // Public methods for the dataTask to interact with the screen
    void setAnalysisResults(double mean, double min, double max, double pk_pk, double std_dev, const std::vector<double>& samples);
    void setSamplingProgress(int percent);
    bool isSampling() const;

private:
    enum class AnalysisState {
        SELECT_SOURCE,
        SAMPLING,
        VIEW_RESULTS
    };

    void handleSelectSourceInput(const InputEvent& event);
    void handleViewResultsInput(const InputEvent& event);
    void getSelectSourceRenderProps(UIRenderProps* props_to_fill);
    void getSamplingRenderProps(UIRenderProps* props_to_fill);
    void getViewResultsRenderProps(UIRenderProps* props_to_fill);

    // --- DEFINITIVE REFACTOR: AdcManager pointer is removed ---
    PBiosContext* _context;
    AnalysisState _current_state;

    std::vector<std::string> _source_menu_items;
    int _selected_source_index;

    int _sampling_progress_percent;
    double _result_samples[ANALYSIS_SAMPLE_COUNT];
    double _result_mean;
    double _result_min;
    double _result_max;
    double _result_pk_pk;
    double _result_std_dev;
};

#endif // NOISE_ANALYSIS_SCREEN_H