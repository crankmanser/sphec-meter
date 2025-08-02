// File Path: /src/ui/StateManager.h
// MODIFIED FILE

#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <map>
#include "InputManager.h"
#include "UIManager.h"

class StateManager;

enum class ScreenState {
    NONE,
    MAIN_MENU,
    PBIOS_MENU,
    FILTER_SELECTION,
    LIVE_FILTER_TUNING,
    PARAMETER_EDIT,
    NOISE_ANALYSIS,
    DRIFT_TRENDING // <<< NEW: Add the new screen state
};

// ... (rest of StateManager.h is unchanged) ...
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
    ScreenState getActiveScreenState() const;
    UIRenderProps* getUiRenderProps();
private:
    std::map<ScreenState, Screen*> _screens;
    Screen* _activeScreen;
    ScreenState _currentState;
    UIRenderProps* _uiRenderProps;
};

#endif // STATE_MANAGER_H