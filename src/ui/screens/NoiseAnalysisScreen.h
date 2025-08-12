// File Path: /src/ui/screens/NoiseAnalysisScreen.h
// MODIFIED FILE

#ifndef NOISE_ANALYSIS_SCREEN_H
#define NOISE_ANALYSIS_SCREEN_H

#include "ui/StateManager.h"
#include <vector>
#include <string>

#define ANALYSIS_SAMPLE_COUNT 512

struct PBiosContext;
class AdcManager;

class NoiseAnalysisScreen : public Screen {
public:
    NoiseAnalysisScreen(PBiosContext* context, AdcManager* adcManager);
    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;
    // --- DEFINITIVE FIX: Update signature to match the base class ---
    void onEnter(StateManager* stateManager, int context = 0) override;

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

    PBiosContext* _context;
    AdcManager* _adcManager;
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