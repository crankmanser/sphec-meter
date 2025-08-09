// File Path: /src/ui/screens/LiveFilterTuningScreen.h
// MODIFIED FILE

#ifndef LIVE_FILTER_TUNING_SCREEN_H
#define LIVE_FILTER_TUNING_SCREEN_H

#include "ui/StateManager.h"
#include <vector>
#include <string>
#include "FilterManager.h"
#include "ui/blocks/GraphBlock.h"

class AdcManager;
class CalibrationManager;
class TempManager;
struct PBiosContext;

class LiveFilterTuningScreen : public Screen {
public:
    LiveFilterTuningScreen(AdcManager* adcManager, PBiosContext* context, CalibrationManager* phCal, CalibrationManager* ecCal, TempManager* tempManager);
    
    void onEnter(StateManager* stateManager) override;
    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;
    void update();

    void getManualTuneRenderProps(UIRenderProps* props_to_fill);

private:
    // --- DEFINITIVE FIX: Restore the state variable for input delegation ---
    bool _is_in_manual_tune_mode;

    void handleHubMenuInput(const InputEvent& event);
    void getHubMenuRenderProps(UIRenderProps* props_to_fill);
    
    AdcManager* _adcManager;
    PBiosContext* _context;
    CalibrationManager* _phCalManager;
    CalibrationManager* _ecCalManager;
    TempManager* _tempManager;

    double _hf_raw_buffer[GRAPH_DATA_POINTS];
    double _hf_filtered_buffer[GRAPH_DATA_POINTS];
    double _lf_filtered_buffer[GRAPH_DATA_POINTS];
    double _ghost_lf_filtered_buffer[GRAPH_DATA_POINTS];
    bool _is_compare_mode_active;
    double _hf_f_std, _hf_r_std, _lf_f_std, _lf_r_std;
    int _hf_stab_percent, _lf_stab_percent;
    double _calibrated_value;

    std::vector<std::string> _hub_menu_items;
    std::vector<std::string> _hub_menu_descriptions;
    int _selected_index;

    FilterManager _saved_tune_snapshot;
};

#endif // LIVE_FILTER_TUNING_SCREEN_H