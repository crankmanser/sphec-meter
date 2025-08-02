// File Path: /src/ui/screens/NoiseAnalysisScreen.h
// MODIFIED FILE

#ifndef NOISE_ANALYSIS_SCREEN_H
#define NOISE_ANALYSIS_SCREEN_H

#include "ui/StateManager.h"
#include <vector>
#include <string>

// --- NEW: Define a constant for the number of samples ---
// This must be a power of 2 for the FFT algorithm to work efficiently.
#define ANALYSIS_SAMPLE_COUNT 512

struct PBiosContext;
class AdcManager;

class NoiseAnalysisScreen : public Screen {
public:
    NoiseAnalysisScreen(PBiosContext* context, AdcManager* adcManager);
    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;
    void onEnter(StateManager* stateManager) override;

    // --- FIX: Signature updated to accept the raw sample data buffer ---
    void setAnalysisResults(double mean, double min, double max, double pk_pk, double std_dev, const std::vector<double>& samples);
    
    // --- NEW: Public method for the data task to update progress ---
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

    PBiosContext* _context;
    AdcManager* _adcManager;
    AnalysisState _current_state;

    std::vector<std::string> _source_menu_items;
    int _selected_source_index;

    // --- NEW: Member variables for progress and graph data ---
    int _sampling_progress_percent;
    double _result_samples[ANALYSIS_SAMPLE_COUNT]; // Fixed-size array for graph data

    double _result_mean;
    double _result_min;
    double _result_max;
    double _result_pk_pk;
    double _result_std_dev;
};

#endif // NOISE_ANALYSIS_SCREEN_H