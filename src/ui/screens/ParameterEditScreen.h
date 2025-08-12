// File Path: /src/ui/screens/ParameterEditScreen.h
// MODIFIED FILE

#ifndef PARAMETER_EDIT_SCREEN_H
#define PARAMETER_EDIT_SCREEN_H

#include "ui/StateManager.h"
#include "pBiosContext.h"
#include "FilterManager.h"

class ParameterEditScreen : public Screen {
public:
    ParameterEditScreen(PBiosContext* context);
    // --- DEFINITIVE FIX: Update signature to match the base class ---
    void onEnter(StateManager* stateManager, int context = 0) override;
    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;

private:
    std::string getSelectedParamValueString();

    PBiosContext* _context;
    int _selected_index;
    bool _is_editing;
    
    // Snapshot for the "Cancel" feature
    PI_Filter _hf_snapshot;
    PI_Filter _lf_snapshot;
    
    std::vector<std::string> _param_menu_items;
};

#endif // PARAMETER_EDIT_SCREEN_H