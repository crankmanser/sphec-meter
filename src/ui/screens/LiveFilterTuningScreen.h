// File Path: /src/ui/screens/LiveFilterTuningScreen.h
// MODIFIED FILE

#ifndef LIVE_FILTER_TUNING_SCREEN_H
#define LIVE_FILTER_TUNING_SCREEN_H

#include "ui/StateManager.h"
#include <vector>
#include <string>
#include "FilterManager.h"
#include "AdcManager.h"
#include "ui/blocks/GraphBlock.h"

// Forward declare the context struct
struct PBiosContext;

/**
 * @class LiveFilterTuningScreen
 * @brief The primary pBIOS screen for real-time digital filter tuning.
 *
 * This screen provides a powerful, data-driven interface for adjusting the
 * parameters of the two-stage (HF/LF) filter pipeline. It visualizes the
 * raw and filtered signals on graphs and displays quantitative Key Performance
 * Indicators (KPIs) to help the user achieve an optimal balance between
 * stability and responsiveness, as outlined in the user manual.
 */
class LiveFilterTuningScreen : public Screen {
public:
    LiveFilterTuningScreen(AdcManager* adcManager, PBiosContext* context);
    
    void onEnter(StateManager* stateManager) override;
    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;

    /**
     * @brief The core data processing loop for this screen.
     * Called by the pBiosDataTask to continuously acquire raw ADC data,
     * pass it through the selected filter pipeline, and update the internal
     * data buffers and KPI values for rendering.
     */
    void update();

    // Public getters for the main loop to configure the edit screen
    const std::string& getSelectedParamName() const;
    int getSelectedParamIndex() const;

private:
    // Helper method to get the string representation of a parameter's value
    std::string getParamValueString(int index);

    AdcManager* _adcManager;
    PBiosContext* _context;

    // Data buffers for the graphs
    double _hf_raw_buffer[GRAPH_DATA_POINTS];
    double _hf_filtered_buffer[GRAPH_DATA_POINTS];
    double _lf_filtered_buffer[GRAPH_DATA_POINTS];

    bool _is_editing;
    
    // Menu items and state
    std::vector<std::string> _menu_item_names;
    int _selected_index;

    // --- NEW: Live KPI Values ---
    // These variables store the latest calculated performance metrics from the
    // PI_Filter instances, ready to be displayed on the UI.
    double _hf_f_std;         // Filtered Standard Deviation for the HF stage
    double _hf_r_std;         // Raw Standard Deviation for the HF stage
    int _hf_stab_percent;     // Stability percentage for the HF stage
    double _lf_f_std;         // Filtered Standard Deviation for the LF stage
    double _lf_r_std;         // Raw Standard Deviation for the LF stage
    int _lf_stab_percent;     // Stability percentage for the LF stage
};

#endif // LIVE_FILTER_TUNING_SCREEN_H