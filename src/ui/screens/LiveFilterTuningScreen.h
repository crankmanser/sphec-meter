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

class LiveFilterTuningScreen : public Screen {
public:
    LiveFilterTuningScreen(AdcManager* adcManager, PBiosContext* context);
    
    void onEnter(StateManager* stateManager) override;
    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;
    void update();

    // --- NEW: Public getters for the main loop to configure the edit screen ---
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

    // --- NEW: State for edit mode ---
    bool _is_editing;
    
    // Menu items and state
    std::vector<std::string> _menu_item_names; // Base names like "HF Settle Threshold"
    int _selected_index;

    // KPI values
    double _hf_f_std;
    double _hf_r_std;
    int _hf_stab_percent;
    double _lf_f_std;
    double _lf_r_std;
    int _lf_stab_percent;
};

#endif // LIVE_FILTER_TUNING_SCREEN_H