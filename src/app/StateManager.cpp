// src/app/StateManager.cpp
// MODIFIED FILE
#include "app/StateManager.h"
#include "DebugMacros.h"
#include <Arduino.h> 

StateManager::StateManager() {}

StateManager::~StateManager() {
    for (std::map<ScreenState, Screen*>::iterator it = _screens.begin(); it != _screens.end(); ++it) {
        delete it->second;
    }
    _screens.clear();
}

// <<< MODIFIED: begin now requires the AppContext >>>
void StateManager::begin(AppContext* context) {
    LOG_MANAGER("StateManager initialized.\n");
    _appContext = context; // Store the context
    if (!_screens.empty() && _screens.count(_currentState)) {
        changeState(_currentState);
    }
}

void StateManager::addScreen(ScreenState state, Screen* screen) {
    _screens[state] = screen;
}

void StateManager::changeState(ScreenState new_state) {
    if (_activeScreen) {
        _activeScreen->onExit();
    }

    if (_screens.count(new_state)) {
        _currentState = new_state;
        _activeScreen = _screens[new_state];
        // <<< MODIFIED: Pass the stored context to the new screen >>>
        _activeScreen->onEnter(this, _appContext);
        LOG_MANAGER("Changed screen state to %d\n", (int)new_state);
    } else {
        LOG_MAIN("[SM_ERROR] Attempted to change to an unknown screen state: %d\n", (int)new_state);
    }
}

Screen* StateManager::getActiveScreen() {
    return _activeScreen;
}