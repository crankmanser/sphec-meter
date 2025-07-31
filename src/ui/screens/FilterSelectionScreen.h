// File Path: /src/ui/screens/FilterSelectionScreen.h
// MODIFIED FILE

#ifndef FILTER_SELECTION_SCREEN_H
#define FILTER_SELECTION_SCREEN_H

#include "ui/StateManager.h"
#include <vector>
#include <string>

// Forward declare the context struct to avoid circular dependencies
struct PBiosContext;

/**
 * @class FilterSelectionScreen
 * @brief A simple menu screen to allow the user to select which filter
 * instance they want to tune in the pBIOS.
 */
class FilterSelectionScreen : public Screen {
public:
    // The constructor now accepts a pointer to the shared pBiosContext
    FilterSelectionScreen(PBiosContext* context);
    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;

private:
    PBiosContext* _context; // Pointer to the shared context
    std::vector<std::string> _menu_items;
    int _selected_index;
};

#endif // FILTER_SELECTION_SCREEN_H