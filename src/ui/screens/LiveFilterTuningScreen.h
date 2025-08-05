// File Path: /src/ui/screens/LiveFilterTuningScreen.h
// MODIFIED FILE

#ifndef LIVE_FILTER_TUNING_SCREEN_H
#define LIVE_FILTER_TUNING_SCREEN_H

#include "ui/StateManager.h"
#include <vector>
#include <string>
#include "FilterManager.h"
#include "ui/blocks/GraphBlock.h"

// --- DEFINITIVE FIX: Forward declare classes to resolve compiler errors ---
class AdcManager;
class CalibrationManager;
class TempManager;
struct PBiosContext;

class LiveFilterTuningScreen : public Screen {
public:
    // This constructor is now correctly declared.
    LiveFilterTuningScreen(AdcManager* adcManager, PBiosContext* context, CalibrationManager* phCal, CalibrationManager* ecCal, TempManager* tempManager);
    
    void onEnter(StateManager* stateManager) override;
    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;
    void update();

    void setCalibratedValue(double value);
    const std::string& getSelectedParamName() const;
    int getSelectedParamIndex() const;

private:
    std::string getSelectedParamValueString();

    AdcManager* _adcManager;
    PBiosContext* _context;
    CalibrationManager* _phCalManager;
    CalibrationManager* _ecCalManager;
    TempManager* _tempManager;

    double _hf_raw_buffer[GRAPH_DATA_POINTS];
    double _hf_filtered_buffer[GRAPH_DATA_POINTS];
    double _lf_filtered_buffer[GRAPH_DATA_POINTS];

    double _hf_f_std, _hf_r_std, _lf_f_std, _lf_r_std;
    int _hf_stab_percent, _lf_stab_percent;
    
    double _calibrated_value;
    std::vector<std::string> _menu_item_names;
    int _selected_index;
};

#endif // LIVE_FILTER_TUNING_SCREEN_H