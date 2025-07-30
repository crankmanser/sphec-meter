// File Path: /lib/StateManager/src/StateManager.cpp
// MODIFIED FILE

#include "StateManager.h"

/**
 * @brief Constructor for the StateManager.
 * Initializes the manager with no active screen.
 */
StateManager::StateManager() :
    _activeScreen(nullptr),
    _currentState(ScreenState::NONE),
    _uiRenderProps(nullptr)
{}

/**
 * @brief Destructor for the StateManager.
 * Cleans up by deleting all screen objects and the shared canvas
 * to prevent memory leaks.
 */
StateManager::~StateManager() {
    // --- FIX: Reverted to a C++11 compatible for loop ---
    for (std::map<ScreenState, Screen*>::iterator it = _screens.begin(); it != _screens.end(); ++it) {
        delete it->second;
    }
    _screens.clear();

    if (_uiRenderProps) {
        delete _uiRenderProps;
        _uiRenderProps = nullptr;
    }
}

/**
 * @brief Initializes the StateManager.
 * This must be called before any other methods. It allocates the
 * shared UIRenderProps object on the heap.
 */
void StateManager::begin() {
    if (!_uiRenderProps) {
        _uiRenderProps = new UIRenderProps();
    }
}

/**
 * @brief Adds a screen to the manager's registry.
 * @param state The ScreenState enum for this screen.
 * @param screen A pointer to the screen object.
 */
void StateManager::addScreen(ScreenState state, Screen* screen) {
    if (_screens.find(state) == _screens.end()) {
        _screens[state] = screen;
    }
}

/**
 * @brief Changes the active screen.
 * Calls onExit() on the current screen and onEnter() on the new screen.
 * @param new_state The ScreenState of the screen to activate.
 */
void StateManager::changeState(ScreenState new_state) {
    if (_activeScreen != nullptr) {
        _activeScreen->onExit();
    }

    auto it = _screens.find(new_state);
    if (it != _screens.end()) {
        _currentState = new_state;
        _activeScreen = it->second;
        _activeScreen->onEnter(this);
    } else {
        // Handle error: screen not found
        _activeScreen = nullptr;
        _currentState = ScreenState::NONE;
    }
}

/**
 * @brief Gets a pointer to the currently active screen.
 * @return A pointer to the active Screen object, or nullptr if none is active.
 */
Screen* StateManager::getActiveScreen() {
    return _activeScreen;
}

/**
 * @brief Gets a pointer to the shared UI render properties object.
 * @return A pointer to the UIRenderProps "Shared Canvas".
 */
UIRenderProps* StateManager::getUiRenderProps() {
    return _uiRenderProps;
}