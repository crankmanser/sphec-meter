// File Path: /src/ui/StateManager.h
// MODIFIED FILE

#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <map>
#include "InputManager.h"
#include "UIManager.h"

class StateManager;

/**
 * @brief --- DEFINITIVE REFACTOR: Simplifies the wizard states. ---
 * The complex multi-stage states are replaced by a single `AUTO_TUNE_RUNNING`
 * state to reflect the new, monolithic `proposeSettings` function.
 */
enum class ScreenState {
    NONE,
    // Main App States
    MAIN_MENU,
    // pBIOS States
    PBIOS_MENU,
    FILTER_SELECTION,
    LIVE_FILTER_TUNING,
    PARAMETER_EDIT,
    AUTO_TUNE_SUB_MENU,
    // --- Wizard State ---
    AUTO_TUNE_RUNNING, // Simplified from four states to one
    // --- Standalone Diagnostic States ---
    NOISE_ANALYSIS,
    DRIFT_TRENDING,
    MAINTENANCE_MENU,
    SHUTDOWN_MENU,
    LIVE_VOLTMETER,
    HARDWARE_SELF_TEST,
    PROBE_PROFILING,
};

class Screen {
public:
    virtual ~Screen() {}
    virtual void onEnter(StateManager* stateManager) { _stateManager = stateManager; }
    virtual void onExit() {}
    virtual void handleInput(const InputEvent& event) = 0;
    virtual void getRenderProps(UIRenderProps* props_to_fill) = 0;
protected:
    StateManager* _stateManager = nullptr;
};

class StateManager {
public:
    StateManager();
    ~StateManager();
    void begin();
    void addScreen(ScreenState state, Screen* screen);
    void changeState(ScreenState new_state);
    Screen* getActiveScreen();
    Screen* getScreen(ScreenState state);
    ScreenState getActiveScreenState() const;
    UIRenderProps* getUiRenderProps();
private:
    std::map<ScreenState, Screen*> _screens;
    Screen* _activeScreen;
    ScreenState _currentState;
    UIRenderProps* _uiRenderProps;
};

#endif // STATE_MANAGER_H