// File Path: /src/ui/screens/LiveFilterTuningScreen.h
// MODIFIED FILE

#ifndef LIVE_FILTER_TUNING_SCREEN_H
#define LIVE_FILTER_TUNING_SCREEN_H

#include "ui/StateManager.h"
#include <vector>
#include <string>
#include "FilterManager.h"
#include "ui/blocks/GraphBlock.h"

// Forward declarations to avoid circular dependencies
class AdcManager;
class CalibrationManager;
class TempManager;
struct PBiosContext;

/**
 * @class LiveFilterTuningScreen
 * @brief The main hub for the "Tuning Workbench" UI.
 *
 * This screen acts as both the primary display for live filter data and the
 * navigation hub for all tuning-related sub-features. It no longer manages
 * the probe's power state directly.
 */
class LiveFilterTuningScreen : public Screen {
public:
    // --- DEFINITIVE REFACTOR: Constructor no longer needs the AdcManager ---
    LiveFilterTuningScreen(PBiosContext* context, CalibrationManager* phCal, CalibrationManager* ecCal, TempManager* tempManager);

    void onEnter(StateManager* stateManager, int context = 0) override;
    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;
    void update();
    void getManualTuneRenderProps(UIRenderProps* props_to_fill);

private:
    void handleHubMenuInput(const InputEvent& event);

    // Pointers to global managers required for data processing
    // --- DEFINITIVE REFACTOR: AdcManager pointer is removed ---
    PBiosContext* _context;
    CalibrationManager* _phCalManager;
    CalibrationManager* _ecCalManager;
    TempManager* _tempManager;

    // Local state and data buffers
    bool _is_in_manual_tune_mode;
    double _hf_raw_buffer[GRAPH_DATA_POINTS];
    double _hf_filtered_buffer[GRAPH_DATA_POINTS];
    double _lf_filtered_buffer[GRAPH_DATA_POINTS];
    double _ghost_lf_filtered_buffer[GRAPH_DATA_POINTS];
    bool _is_compare_mode_active;
    double _hf_f_std, _hf_r_std, _lf_f_std, _lf_r_std;
    int _hf_stab_percent, _lf_stab_percent;
    double _calibrated_value;
    std::vector<std::string> _hub_menu_items;
    int _selected_index;
    FilterManager _saved_tune_snapshot;
};

#endif // LIVE_FILTER_TUNING_SCREEN_H