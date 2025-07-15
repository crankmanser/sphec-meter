# Architecture: Smart-EC-pH-Meter v1.5.0

This document outlines the "Cabinet" software architecture for the v1.5.0 refactor. It is the single source of truth for the project's design.

## 1. Guiding Principles
* **Isolation (The "Cabinet" Philosophy)**: Each component is a black box.
* **Single Responsibility**: Each cabinet has one job.
* **Strict Layering**: Components only interact with layers below them.
* **Code Brevity (The "Anti-God File" Rule)**: Files should be small and focused.

---

## 2. Layered Architecture Overview
The firmware is divided into four distinct layers. Data flows upwards from the hardware to the UI.
* **Layer 1 - HAL (Hardware Abstraction Layer)**
* **Layer 2 - Manager Layer**
* **Layer 3 - Application Layer**
* **Layer 4 - Presentation Layer (UI)**

---

## 3. UI Architecture: Block-Based Assembly

The UI is built on a **declarative, block-based model**. Screens are simple state machines that contain no drawing logic. A screen's job is to select the appropriate UI "Block" (e.g., `MenuBlock`, `GraphBlock`) and provide it with data. The `UIManager` is the rendering engine that calls the correct block's `draw()` method. This enforces a high degree of separation and reusability.

The **button prompt system** is standardized. Labels are drawn on the OLED adjacent to the corresponding physical button's counterpart on the opposite side of the device:
* **Top Button** -> Label on **OLED #3** (Bottom)
* **Middle Button** -> Label on **OLED #2** (Middle)
* **Bottom Button** -> Label on **OLED #1** (Top)

sphec_meter_v1.4.7/  
├── src/  
│   ├── main.cpp               # Entry point: minimal setup()/loop()  
│   ├── hal/  
│   │   ├── ... (HAL Drivers)  
│   ├── managers/  
│   │   ├── power/  
│   │   ├── storage/  
│   │   │   ├── diagnostics/  
│   │   │   │   ├── StorageDiagnostics.h  
│   │   │   │   └── StorageDiagnostics.cpp  
│   │   │   ├── helpers/  
│   │   │   │   ├── AtomicSave.h  
│   │   │   │   ├── AtomicSave.cpp  
│   │   │   │   ├── FileNamer.h  
│   │   │   │   └── FileNamer.cpp  
│   │   │   ├── StorageManager.h  
│   │   │   └── StorageManager.cpp  
│   │   ├── connectivity/  
│   │   ├── sensor/  
│   │   │   ├── calibration/  
│   │   │   │   ├── CalibrationSessionManager.h  
│   │   │   │   └── CalibrationSessionManager.cpp  
│   │   │   ├── SensorProcessor.h  
│   │   │   └── SensorProcessor.cpp  
│   │   ├── ... (Other Managers)  
│   ├── app/  
│   │   ├── api_handlers/  
│   │   │   ├── CalibrationHandlers.h  
│   │   │   ├── CalibrationHandlers.cpp  
│   │   │   ├── TuningHandlers.h  
│   │   │   ├── TuningHandlers.cpp  
│   │   │   ├── StorageHandlers.h  
│   │   │   ├── StorageHandlers.cpp  
│   │   │   ├── SensorDataHandlers.h  
│   │   │   └── SensorDataHandlers.cpp  
│   │   ├── WebService.h  
│   │   ├── WebService.cpp  
│   │   ├── ... (Other App Components & Tasks)  
│   ├── helpers/  
│   │   ├── ... (Filters, Calibration Helpers, etc.)  
│   └── ...  
├── ... (Other project files)


## 4. RTOS Design & Concurrency

The firmware is a true multi-threaded application.

* **Core Affinity**: To guarantee UI responsiveness, tasks are pinned to specific CPU cores.
    * **Core 1 (UI & Input Core)**: Hosts the `UiTask` and the high-priority `EncoderTask`. This creates a low-latency loop for user interaction.
    * **Core 0 (Processing Core)**: Hosts all other background tasks, including `SensorTask`, `TelemetryTask`, `ConnectivityTask`, and `StorageTask`.

* **Event-Driven Input**: The rotary encoder is serviced by a hardware ISR that places raw state changes onto a FreeRTOS queue. The dedicated `EncoderTask` processes these events and applies a "Speed Engine" to translate them into smooth UI steps. This decouples input capture from the UI rendering loop, eliminating jitter.

* **Focused Analysis Mode**: For computationally expensive diagnostics, the system can enter a mode where non-essential background tasks are temporarily suspended via `vTaskSuspend()`. This dedicates maximum CPU resources to the analysis and ensures the highest possible signal integrity during sensitive measurements.


### 4.1. Core #1: The GUI Engine (The "Canvas")
* **Purpose**: This is the foundational rendering and input pipeline.
* **Components**: `DisplayManager` (HAL), `UIManager` (Renderer), `UiTask` (Orchestrator).
* **Behavior**: The `UiTask` orchestrates the process of getting input, retrieving declarative render properties from the active screen, and commanding the `UIManager` to draw the final frame.

### 4.2. Core #2: The Stateful Status System (The "Dashboard")
* **Purpose**: To manage the globally visible status areas independently of the main screen content.
* **Components**: `StatusIndicatorController`, `NotificationManager`.
* **Behavior**: The `StatusIndicatorController` observes the state of various system managers (e.g., `PowerManager`, `WifiManager`) and produces a set of `TopStatusProps` and `StateStackProps`. These props define the icons to be displayed in the two status areas: the **System Tray** (top of OLEDs #1 & #2) and the **State Stack** (left of OLED #3).

### 4.3. Core #3: The Wizard Engine (The "Director")
* **Purpose**: A generic, reusable framework for handling multi-step, sequential user workflows.
* **Components**: `Wizard` base class, `WizardStep` base class, and a generic `WizardRunnerScreen`.
* **Behavior**: A specific wizard (e.g., `PhCalibrationWizard`) is constructed from a sequence of specialized `WizardStep` objects. The `WizardRunnerScreen` hosts and runs the wizard, allowing for the easy creation of complex guided processes without creating monolithic screen files.

### 4.4. Core #4: The Graphing & Trending Engine (The "Chartist")
* **Purpose**: To provide a powerful, reusable system for drawing time-series graphs.
* **Components**: A `Graphing` helper library (for managing data buffers and series) and dedicated rendering functions within the `UIManager`.
* **Behavior**: A screen wishing to display a graph will manage the `GraphDataBuffer` and describe the graph's appearance via a `GraphProps` data structure, which the `UIManager` then renders.


## 5. Boot & Execution Flow

The system's execution flow is now controlled by a minimal bootloader in `main.cpp`.

5.1.  **`main.cpp` (Bootloader)**:
    * Initializes core peripherals (`init_globals`, `init_i2c_devices`).
    * Instantiates all possible manager objects.
    * Initializes the `StorageManager`.
    * Checks for a clean shutdown flag. If the flag is not present, it runs the `StorageManager`'s asynchronous recovery routine.
    * Detects the user's requested boot mode (`NORMAL` or `DIAGNOSTICS`) by reading the hardware buttons.
    * Calls either `run_normal_mode()` or `run_pbios_mode()`, handing off control permanently.

5.2.  **`run_normal_mode()` (Application Mode Handler)**:
    * Initializes all required hardware abstraction layers (`init_hals`).
    * Initializes the `ButtonManager` and `EncoderManager`.
    * Creates the full set of FreeRTOS tasks (`init_tasks`).
    * Initializes the full set of application managers (`init_managers`).
    * Yields control entirely to the FreeRTOS scheduler, which now runs the main application.

5.3.  **`run_pbios_mode()` (Diagnostics Mode Handler)**:
    * Initializes only the minimal set of managers required for the diagnostics UI (`ButtonManager`, `UIManager`, etc.).
    * **Does not** create the main application's RTOS tasks.
    * Enters a simple, blocking `while(true)` loop that polls for input and renders the pBios UI, providing a stable and quiet environment for running tests.

