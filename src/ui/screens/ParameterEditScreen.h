// File Path: /src/ui/screens/ParameterEditScreen.h
// MODIFIED FILE

#ifndef PARAMETER_EDIT_SCREEN_H
#define PARAMETER_EDIT_SCREEN_H

#include "ui/StateManager.h"
#include <string>

struct PBiosContext;

class ParameterEditScreen : public Screen {
public:
    ParameterEditScreen(PBiosContext* context);
    void handleInput(const InputEvent& event) override;
    
    // --- FIX: This function is now an "overlay" renderer ---
    // It will only modify the props for the middle OLED, leaving the others untouched.
    void getRenderProps(UIRenderProps* props_to_fill) override;

    void setParameterToEdit(const std::string& name, int index);

private:
    std::string getParamValueString();

    PBiosContext* _context;
    std::string _param_name;
    int _param_index;
};

#endif // PARAMETER_EDIT_SCREEN_H