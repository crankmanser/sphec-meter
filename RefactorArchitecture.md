Architecture: Smart-EC-pH-Meter v1.4.7
This document outlines the "Cabinet" software architecture for the v1.4.7 refactor. It is the single source of truth for the project's design, defining the strict layers, components, and design patterns that must be followed to ensure a stable, maintainable, and robust firmware.

1. Guiding Principles
⦁	Isolation (The "Cabinet" Philosophy): Each component (cabinet) is a black box. It hides its internal complexity and exposes a clean, simple public interface. Cabinets must not expose their internal state or dependencies.
⦁	Single Responsibility: Each cabinet has one, and only one, job. For example, the PowerManager's only job is to manage the battery state; it does not handle Wi-Fi or sensor readings.
⦁	Strict Layering: Components can only interact with components in the layers below them. An Application component can call a Manager, but a Manager cannot call an Application component. This enforces a one-way flow of dependencies.
⦁	Code Brevity (The "Anti-God File" Rule): To ensure maintainability and readability, any source file approaching or exceeding 200 lines of code must be critically reviewed for refactoring. Complex logic within a file should be broken down into smaller, logical functions or extracted into new, dedicated helper classes and consolidated into appropriate sub-directories.
⦁	Standardization Over Flexibility: To ensure a consistent and predictable user experience, the UI is built from a fixed set of standardized layouts and components.

2. Layered Architecture Overview
The firmware is divided into four distinct layers. Data flows upwards from the hardware to the UI.
⦁	Layer 1 - HAL (Hardware Abstraction Layer): Direct, stateless drivers for physical hardware components.
⦁	Layer 2 - Manager Layer: The "brains" of the operation. Contains all the core logic, state management, and services, each encapsulated in its own cabinet.
⦁	Layer 3 - Application Layer: Orchestrates the managers to create a functioning application. Contains all high-level logic, RTOS tasks, and the system state machine.
⦁	Layer 4 - Presentation Layer (UI): Responsible for displaying data and presenting the UI to the user. Contains no business logic.

3. Proposed File Structure
This file structure physically enforces the layered architecture and the new code brevity principle.

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


## 4. UI Architecture: The Four-Core Model

To ensure a modular, responsive, and maintainable user interface, the UI is not a monolithic system. It is built from a set of four distinct, cooperative "cores," each with a single responsibility. This design is detailed further in the **UIManifest.md**.

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



