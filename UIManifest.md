# SpHEC Meter: UI Manifest & Developer Guide

**Version:** 1.0
**Status:** Architecture Finalized

This document is the **single source of truth** for the SpHEC Meter's UI architecture. It is essential reading for any developer working on the user interface. Its purpose is to ensure all contributions adhere to the established design patterns, maintaining the system's modularity and preventing the creation of "god files."

---


## 1. The UI Philosophy: Block-Based Assembly

The UI is built on one core principle: **Screens do not draw themselves.**

A screen's only job is to act as a **state machine**. It responds to user input and manages its internal state (e.g., the selected menu item). To render itself, it populates a properties struct (`UIRenderProps`) that describes which reusable **UI Blocks** to use (e.g., `MenuBlock`, `GraphBlock`) and what data to display.

The `UIManager` is the only component that performs drawing. It takes the `UIRenderProps` from the active screen and calls the appropriate `draw()` method for each requested block. This strict separation of concerns is the primary defense against monolithic "god files" and ensures a consistent look and feel.


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

1.  **Create Directory:** Create a new directory for your screen that follows the menu hierarchy (e.g., `src/presentation/screens/main_menu/settings/`).
2.  **Create the Screen Class**: Create new `.h` and `.cpp` files for your screen. The class must inherit from the `Screen` base class.
3.  **Implement State Logic**: In the `.cpp` file, implement the logic to manage the screen's internal state (e.g., a list of menu items and an integer to track the selected index).
4.  **Implement `handleInput()`**: Write the logic to change the screen's state based on input events from the user.
5.  **Implement `getRenderProps()`**: This is the most important method.
    * Create a new `UIRenderProps` object.
    * **Populate the Block Props**: Instead of drawing, you will populate the properties for the UI Block you want to use. For a menu, you would enable the `menu_props` on the desired OLED and provide it with your list of items and the selected index.
    * **Populate Button Prompts**: Set the text for the button prompts (`top_button_text`, `middle_button_text`, `bottom_button_text`).
    * Return the completed `props` object.
6.  **Register the Screen**:
    * Add a new state to the `ScreenState` enum in `src/app/common/App_types.h`.
    * In `src/boot/init_managers.cpp`, create an instance of your new screen and register it with the `StateManager` using `stateManager->addScreen(...)`.

By following these steps, you contribute a new, modular component that integrates seamlessly into the existing architecture without violating its core principles.


## 1. The UI Philosophy: Block-Based Assembly

The UI is built on one core principle: **Screens do not draw themselves.**

A screen's only job is to act as a **state machine**. It responds to user input and manages its internal state. To render itself, it populates a properties struct (`UIRenderProps`) that describes which reusable **UI Blocks** to use (e.g., `MenuBlock`, `GraphBlock`) and what data to display. The `UIManager` is the only component that performs drawing during normal operation.

---

## 2. Boot & Shutdown UI

The system utilizes dedicated UI blocks for boot and shutdown sequences that can be called directly, even before the main `UIManager` and RTOS tasks are running.

* **`BootBlock`**: A specialized block used by the bootloader in `main.cpp`. It is responsible for displaying the initial boot status, the interactive "pBios" prompt, and the "Recovery Mode" screen.
* **`ShutdownBlock`**: A simple, full-screen block used by the `initiate_shutdown()` handler to display a "Shutting Down" message, ensuring the user knows the device has received the command.

---

## 3. The Four-Core Architecture (Main Application)

The UI of the main application is composed of four distinct, cooperative systems.

### Core #1: The GUI Engine (The "Canvas")
* **`UiTask`**: The RTOS task that orchestrates the entire process.
* **`StateManager`**: Owns all screen instances and manages transitions.
* **`UIManager`**: The rendering engine. Takes `UIRenderProps` and draws the frame.

### Core #2: The Stateful Status System (The "Dashboard")
* **`StatusIndicatorController`**: The "brain" that observes system state and decides which status icons to display.

### Core #3: The Wizard Engine (The "Director")
* **`Wizard` & `WizardStep`**: A framework for creating multi-step, guided workflows like calibration.

### Core #4: The Graphing & Trending Engine (The "Chartist")
* **`GraphDataBuffer` & `GraphProps`**: A system for managing and displaying time-series data.

---
