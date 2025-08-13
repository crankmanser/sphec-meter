// File Path: /src/ui/screens/ProbeHealthCheckScreen.h
// NEW FILE

#ifndef PROBE_HEALTH_CHECK_SCREEN_H
#define PROBE_HEALTH_CHECK_SCREEN_H

#include "ui/StateManager.h"
#include <vector>
#include <string>

/**
 * @class ProbeHealthCheckScreen
 * @brief A screen for performing a quick 1-point probe health check.
 */
class ProbeHealthCheckScreen : public Screen {
public:
    ProbeHealthCheckScreen();
    void onEnter(StateManager* stateManager, int context = 0) override;
    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;

private:
    enum class HealthCheckState {
        SELECT_PROBE,
        MEASURING,
        VIEW_RESULT
    };
    
    HealthCheckState _state;
    std::vector<std::string> _menu_items;
    int _selected_index;
    int _live_stability_percent;
    double _health_result_percent;
};

#endif // PROBE_HEALTH_CHECK_SCREEN_H