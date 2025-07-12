# SpHEC Meter: UI Manifest & Developer Guide

**Version:** 1.0
**Status:** Architecture Finalized

This document is the **single source of truth** for the SpHEC Meter's UI architecture. It is essential reading for any developer working on the user interface. Its purpose is to ensure all contributions adhere to the established design patterns, maintaining the system's modularity and preventing the creation of "god files."

---

## 1. The UI Philosophy: Declarative & Component-Based

The UI is built on one core principle: **Screens do not draw themselves.**

A screen's only job is to act as a **state machine**. It responds to user input, manages its internal state (e.g., the selected menu item), and, when asked, produces a simple, declarative data structure (`UIRenderProps`) that describes *what* should be on the screen.

The `UIManager` is the only component that performs drawing. It takes the `UIRenderProps` and translates that data into low-level draw commands. This strict separation of concerns is the primary defense against "god files."

---

## 2. The Four-Core Architecture

The UI is composed of four distinct, cooperative systems.

### Core #1: The GUI Engine (The "Canvas")
This is the foundational rendering loop.
* **`UiTask`**: The RTOS task that orchestrates the entire process. In a tight loop, it gets input, asks the active screen for its render props, and passes them to the `UIManager`.
* **`StateManager`**: Owns all screen instances and manages the `_activeScreen` pointer. Handles transitions between screens.
* **`UIManager`**: The rendering engine. Takes `UIRenderProps` and draws the frame. It contains no application logic.

### Core #2: The Stateful Status System (The "Dashboard")
This system manages the two status bars.
* **`StatusIndicatorController`**: The "brain." It observes system managers (`PowerManager`, `WifiManager`, etc.) and a `NotificationManager`. It uses internal, prioritized state machines to decide which status icons to display.
* **System Tray (Top of OLEDs #1 & #2)**: A non-persistent bar for background service status (Wi-Fi, SD Card) and component health (Probes, Power Buses).
* **State Stack (Left of OLED #3)**: A persistent, prioritized list of the top 2-3 most critical system-wide events (e.g., Error, Calibrating, Logging).

### Core #3: The Wizard Engine (The "Director")
This system provides a framework for creating multi-step, guided workflows.
* **`Wizard` & `WizardStep`**: To create a new wizard (e.g., for a new type of calibration), you do not create a new, monolithic screen. Instead, you create a new class inheriting from `Wizard` and give it a sequence of `WizardStep` objects.
* **`WizardRunnerScreen`**: A generic screen that can host and run any `Wizard` object, handling the flow control (`next`, `back`, `cancel`).

### Core #4: The Graphing & Trending Engine (The "Chartist")
This system handles all data visualization.
* **`GraphDataBuffer`**: A helper class for managing time-series data.
* **`GraphProps`**: A declarative struct that a screen uses to describe a graph (axes, series, colors, etc.).
* **`UIManager`**: Contains the internal rendering logic to draw a graph based on the `GraphProps`.

---

## 3. How to Add a New Screen

To add a new screen to the UI (e.g., a "Settings" menu):

1.  **Create the Screen Class**: Create new files `src/presentation/screens/SettingsScreen.h` and `.cpp`. The class must inherit from the `Screen` base class.
2.  **Implement State Logic**: In `SettingsScreen.cpp`, implement the logic to manage the screen's state. For a menu, this would be the list of menu items and an integer to track the selected index.
3.  **Implement `handleInput()`**: Write the logic to change the screen's state based on input events. An `ENCODER_INCREMENT` event should increment the selected index. A `BTN_MIDDLE_PRESS` event should call `_stateManager->changeState(...)` to navigate to the selected sub-menu.
4.  **Implement `getRenderProps()`**: This is the most important method.
    * Create a new `UIRenderProps` object.
    * Populate the `OledProps` for the middle screen using the **block-based system**. For a menu, you would create `MenuLine` objects for the previous, selected, and next items. The `is_selected` flag should be set to `true` for the currently selected item.
    * Populate the `button_prompts` field with the appropriate text ("Select", "Back", etc.).
    * Return the completed `props` object.
5.  **Register the Screen**:
    * Add a new state to the `ScreenState` enum in `src/app/common/App_types.h`.
    * In `main.cpp`, create an instance of your new `SettingsScreen` and register it with the `StateManager` using `stateManager->addScreen(...)`.

By following these steps, you contribute a new, modular component that integrates seamlessly into the existing architecture without violating its core principles.
