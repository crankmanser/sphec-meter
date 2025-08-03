// File Path: /src/ui/screens/LiveFilterTuningScreen.h
// MODIFIED FILE

#ifndef LIVE_FILTER_TUNING_SCREEN_H
#define LIVE_FILTER_TUNING_SCREEN_H

#include "ui/StateManager.h"
#include <vector>
#include <string>
#include "FilterManager.h"
#include "AdcManager.h"
#include "CalibrationManager.h" // <<< NEW: Include for calibration logic
#include "TempManager.h"      // <<< NEW: Include for temperature compensation
#include "ui/blocks/GraphBlock.h"

// Forward declare the context struct
struct PBiosContext;

class LiveFilterTuningScreen : public Screen {
public:
    // --- FIX: Constructor now requires the calibration and temperature managers ---
    LiveFilterTuningScreen(AdcManager* adcManager, PBiosContext* context, CalibrationManager* phCal, CalibrationManager* ecCal, TempManager* tempManager);
    
    void onEnter(StateManager* stateManager) override;
    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;
    void update();

    // --- NEW: Public method for the data task to push the live calibrated value ---
    void setCalibratedValue(double value);

    const std::string& getSelectedParamName() const;
    int getSelectedParamIndex() const;

private:
    std::string getParamValueString(int index);

    AdcManager* _adcManager;
    PBiosContext* _context;
    // --- NEW: Pointers to the required managers for calibration ---
    CalibrationManager* _phCalManager;
    CalibrationManager* _ecCalManager;
    TempManager* _tempManager;

    double _hf_raw_buffer[GRAPH_DATA_POINTS];
    double _hf_filtered_buffer[GRAPH_DATA_POINTS];
    double _lf_filtered_buffer[GRAPH_DATA_POINTS];

    bool _is_editing;
    std::vector<std::string> _menu_item_names;
    int _selected_index;

    // --- NEW: Member variable to store the live calibrated value ---
    double _calibrated_value;

    double _hf_f_std;
    double _hf_r_std;
    int _hf_stab_percent;
    double _lf_f_std;
    double _lf_r_std;
    int _lf_stab_percent;
};

#endif // LIVE_FILTER_TUNING_SCREEN_H