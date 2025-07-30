// File Path: /lib/StateManager/src/StateManager.h
// MODIFIED FILE

#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <map>
#include "InputManager.h" // For InputEvent
#include "UIManager.h"    // For UIRenderProps

// Forward-declare dependencies to keep the header clean.
class StateManager;

/**
 * @enum ScreenState
 * @brief Defines all possible screens in the application.
 * This enum is used as a key to identify and switch between screens.
 */
enum class ScreenState {
    NONE,
    MAIN_MENU,
    PBIOS_MENU,
    // ... other screens will be added here
};

/**
 * @class Screen
 * @brief An abstract base class for all UI screens.
 *
 * Each concrete screen (e.g., MainMenuScreen) will inherit from this class
 * and implement its virtual methods. This enforces the declarative UI pattern
 * where screens are simple state machines, not drawing canvases.
 */
class Screen {
public:
    virtual ~Screen() {}

    /**
     * @brief Called when the screen becomes active.
     * @param stateManager A pointer to the state manager for screen transitions.
     */
    virtual void onEnter(StateManager* stateManager) {
        _stateManager = stateManager;
    }

    /**
     * @brief Called when the screen is about to be deactivated.
     */
    virtual void onExit() {}

    /**
     * @brief Handles a user input event.
     * The screen's primary logic resides here, where it updates its internal
     * state based on user actions.
     * @param event The input event to process.
     */
    virtual void handleInput(const InputEvent& event) = 0;

    /**
     * @brief Populates the shared render properties structure.
     * This is the core of the "Shared Canvas" pattern. Instead of drawing,
     * the screen "paints" the provided props object to describe what the
     * UIManager should render.
     * @param props_to_fill A pointer to the UIRenderProps object to be filled.
     */
    virtual void getRenderProps(UIRenderProps* props_to_fill) = 0;

protected:
    StateManager* _stateManager = nullptr;
};

/**
 * @class StateManager
 * @brief Manages the state and transitions of the user interface.
 *
 * This cabinet owns all screen objects and controls which screen is currently
 * active. It provides a centralized mechanism for navigating the UI.
 */
class StateManager {
public:
    StateManager();
    ~StateManager();

    /**
     * @brief Initializes the StateManager and creates the shared UI canvas.
     */
    void begin();

    /**
     * @brief Adds a screen to the manager's registry.
     * The StateManager takes ownership of the screen pointer and will delete it.
     * @param state The ScreenState enum for this screen.
     * @param screen A pointer to the screen object.
     */
    void addScreen(ScreenState state, Screen* screen);

    /**
     * @brief Changes the active screen.
     * @param new_state The ScreenState of the screen to activate.
     */
    void changeState(ScreenState new_state);

    /**
     * @brief Gets a pointer to the currently active screen.
     * @return A pointer to the active Screen object.
     */
    Screen* getActiveScreen();

    /**
     * @brief Gets a pointer to the shared UI render properties object.
     * @return A pointer to the UIRenderProps "Shared Canvas".
     */
    UIRenderProps* getUiRenderProps();

private:
    std::map<ScreenState, Screen*> _screens;
    Screen* _activeScreen;
    ScreenState _currentState;
    UIRenderProps* _uiRenderProps; // The "Shared Canvas"
};

#endif // STATE_MANAGER_H