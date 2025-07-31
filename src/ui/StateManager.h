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
    LIVE_FILTER_TUNING
};

class Screen {
public:
    virtual ~Screen() {}
    // The onEnter method now receives a pointer to the StateManager that owns it.
    // This allows a screen to call back to the manager to change the state.
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
    ScreenState getActiveScreenState() const; // Getter for type-safe checks
    UIRenderProps* getUiRenderProps();
private:
    std::map<ScreenState, Screen*> _screens;
    Screen* _activeScreen;
    ScreenState _currentState;
    UIRenderProps* _uiRenderProps;
};

#endif // STATE_MANAGER_H