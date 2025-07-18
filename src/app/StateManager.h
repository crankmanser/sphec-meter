// src/app/StateManager.h
// MODIFIED FILE
#pragma once

#include "presentation/screens/Screen.h"
#include "app/common/App_types.h"
#include <map>

// Forward-declare AppContext
struct AppContext;

class StateManager {
public:
    StateManager();
    ~StateManager();

    // <<< MODIFIED: begin now requires the AppContext >>>
    void begin(AppContext* context);
    void addScreen(ScreenState state, Screen* screen);
    void changeState(ScreenState new_state);
    Screen* getActiveScreen();

private:
    std::map<ScreenState, Screen*> _screens;
    Screen* _activeScreen = nullptr;
    ScreenState _currentState = ScreenState::SCREEN_HOME;
    // <<< ADDED: A pointer to the application context >>>
    AppContext* _appContext = nullptr;
};