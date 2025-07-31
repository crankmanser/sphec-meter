// File Path: /src/ui/screens/LiveFilterTuningScreen.h
// MODIFIED FILE

#ifndef LIVE_FILTER_TUNING_SCREEN_H
#define LIVE_FILTER_TUNING_SCREEN_H

#include "ui/StateManager.h"
#include <vector>
#include <string>
#include "FilterManager.h"
#include "AdcManager.h"
#include "ui/blocks/GraphBlock.h" // <<< FIX: Include for GRAPH_DATA_POINTS

// Forward declare the context struct
struct PBiosContext;

class LiveFilterTuningScreen : public Screen {
public:
    LiveFilterTuningScreen(AdcManager* adcManager, PBiosContext* context);
    
    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;
    void update();

private:
    AdcManager* _adcManager;
    PBiosContext* _context;

    // Data buffers for the graphs
    double _hf_raw_buffer[GRAPH_DATA_POINTS];
    double _hf_filtered_buffer[GRAPH_DATA_POINTS];
    double _lf_filtered_buffer[GRAPH_DATA_POINTS];

    // Menu items for tunable parameters
    std::vector<std::string> _menu_items;
    int _selected_index;

    // TODO: Add member variables for real-time KPI values
};

#endif // LIVE_FILTER_TUNING_SCREEN_H