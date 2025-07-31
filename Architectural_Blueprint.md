# SpHEC Meter - Architectural Blueprint v2.1.0

This document is the "genesis document" and official architectural summary for the SpHEC Meter firmware. It encapsulates all design decisions made during our initial planning phase and serves as the foundational context for all future development.

## 1. Core Philosophy & Architecture

The firmware is built on a **"Cabinet" philosophy**. Each major component (e.g., `PowerMonitor`, `CalibrationManager`) is a self-contained module with a clear responsibility, promoting separation of concerns.

### 1.1. FreeRTOS Task Structure

The system uses a dual-core FreeRTOS architecture to ensure UI responsiveness and operational stability.

| Task          | Core | Priority | Purpose                                                      |
|---------------|:----:|:--------:|--------------------------------------------------------------|
| `uiTask`      | 1    | 3 (High) | Manages all user input, screen state, and display updates.   |
| `adcTask`     | 0    | 2 (Norm) | Dedicated to reading SPI ADCs (ADS1118) to ensure stability. |
| `sdTask`      | 0    | 2 (Norm) | Handles all asynchronous file I/O requests via a queue.      |
| `i2cTask`     | 1    | 1 (Low)  | Periodically reads all I2C sensors (RTC, INA219).            |
| `oneWireTask` | 0    | 1 (Low)  | Periodically reads all 1-Wire sensors (DS18B20, DHT11).      |
| `sensorTask`  | 0    | 1 (Low)  | **Data Aggregator**: Centralizes raw data from other tasks.    |

### 1.2. Inter-Task Communication

* **Asynchronous Request Queues:** Used for slow or blocking operations. The `sdTask` uses a queue to receive file requests, decoupling other tasks from slow SD card I/O.
* **Mutex-Protected Shared Data:** A central `g_processed_data` struct, protected by a mutex, holds the latest aggregated and processed sensor data for the UI to consume.

### 1.3. Key Constraints

* **SPI Bus Precedence Rule:** The `ADS_Manager` (ADC) **MUST** be initialized before the `SdManager` (SD Card) to electrically prime the bus.
* **`sdTask` Priority Inversion Rule:** The `sdTask` **MUST** temporarily elevate its RTOS priority during critical file I/O operations to prevent deadlocks.
* **ADC Priming Read Rule:** The ADC driver **MUST** perform two consecutive reads for any analog probe measurement, discarding the first result to ensure the reading is stable. Failure to do so results in erratic and unreliable raw voltage data.

## 2. System Cabinets & Engines

### 2.1. Filter Engine

* **Design:** A sequential, two-stage filter pipeline processes all analog sensor data. `Raw Data` -> `Filter 1 (HF)` -> `Filter 2 (LF)` -> `Calibrated Output`.
* **Filter 1 (High-Frequency):** A Median filter followed by a PI filter, designed to reject short-term spikes (<1s).
* **Filter 2 (Low-Frequency):** A Median filter followed by a PI filter, designed to smooth slower signal drift (<2min).
* **Tuning:** Filter tuning (`Settle Threshold`, `Lock Smoothing`, etc.) is performed **exclusively** in the **Diagnostics Mode** UI. In Normal Mode, the filters run "headlessly" using parameters loaded from internal flash.
* **Storage:** Filter parameters are stored in the ESP32's internal flash (NVS) via the `ConfigManager` for fast boot-time access.

### 2.2. Calibration Engine ("Smart Calibration")

* **Purpose:** To create a precise mathematical model translating filtered voltage into a scientific measurement (pH/EC), while also tracking probe health.
* **Model:** Uses a **quadratic function (y = ax² + bx + c)** for a more accurate curve of best fit over 3 calibration points. This model is flexible enough to map the linear output of sensor modules (like the PH-4502C) while also capturing non-linearities from external interference.
* **Hybrid Procedure:** A two-stage process is required for modules like the PH-4502C:
    1.  **Hardware Calibration:** A one-time physical adjustment of the sensor module's offset potentiometer to set the output to 2500mV in a neutral pH 7.0 solution.
    2.  **Software Calibration:** The standard 3-point calibration wizard which builds the quadratic model.
* **KPIs:** The engine calculates several Key Performance Indicators:
    * **Calibration Quality Score %:** Grades a new calibration's reliability based on goodness-of-fit (R-squared) and a slope sanity check.
    * **Sensor Drift %:** Measures long-term probe aging by comparing new and previous calibration curves.
    * **Calibration Health %:** Measures short-term drift via a 1-point voltage check.
    * **Zero-Point Drift (mV):** Measures the long-term drift of the hardware's physical offset by tracking the neutral voltage reading across multiple calibrations.
* **Storage:** The rich calibration models are saved to the SD card via the `StorageEngine`.

### 2.3. Power Monitor ("Intelligent Power Monitor")

* **Purpose:** To provide an accurate, KPI-driven view of battery state and health.
* **Hardware:** Uses an INA219 for bidirectional current sensing.
* **Hybrid Model:**
    * **Coulomb Counting:** Precisely tracks energy (Watt-hours) flowing into and out of the battery. A **moving average filter** is applied to the raw current readings to smooth high-frequency fluctuations from the ESP32's radio, improving accuracy.
    * **Voltage Reconciliation:** Uses Open-Circuit Voltage (OCV) to correct for drift after power-off events.
    * **Stateful Learning:** An automated, long-term **SOH Recalibration Cycle** measures the battery's true capacity, making the SOH value progressively more accurate.
* **Storage:** The power monitor's state is persisted to the SD card via the `StorageEngine`.

### 2.4. Storage, Backup, and "Limp Mode"

* **`StorageEngine`:** The **sole, mandatory interface** for all persistent configuration data operations.
* **Atomic Writes:** To prevent data corruption, the engine uses an atomic write procedure: `write to .tmp` -> `delete .bak` -> `rename .json to .bak` -> `rename .tmp to .json`.
* **"Limp Mode":** The storage layer is abstracted via an `I_StorageProvider` interface to handle SD card failure by redirecting file operations to a remote source.

## 3. User Interface Architecture

The User Interface is built upon a robust, modular, and stable architecture designed to ensure responsiveness and prevent the instability issues of the legacy system. It is composed of three distinct, isolated UI systems.

### 3.1. The Three UI Engines

The firmware's UI is not a single monolithic entity but is divided into three purpose-built systems:

1.  **The Boot UI (Self-Contained Bootloader):**
    * **Location:** `src/boot/boot_sequence.cpp`
    * **Responsibility:** To handle the initial boot animation and the critical `NORMAL` vs. `PBIOS` mode selection.
    * **Architecture:** This is not a full "engine" but a simple, self-contained, blocking `while` loop. It performs its own direct, low-level polling of the input pins and draws directly to the screen via the `DisplayManager`. It has **no dependencies** on the main application's RTOS tasks or UI cabinets (`InputManager`, `StateManager`). This extreme simplicity guarantees maximum stability during the critical startup phase.

2.  **The pBIOS UI Engine (Diagnostics Mode):**
    * **Location:** `main.cpp`, within the `else` block after boot mode selection.
    * **Responsibility:** To provide a user interface for the diagnostics (pBios) mode.
    * **Architecture:** A simple, blocking `while` loop that runs directly on Core 0 after `setup()` completes. It instantiates its own `InputManager` for a consistent user feel but does **not** use the `StateManager` or a dedicated `uiTask`. It directly creates and manages its own screen instances (e.g., `pBiosMenuScreen`) and orchestrates the input and rendering loop itself. This keeps it lightweight and isolated from the main application's complexity.

3.  **The Main UI Engine (Normal Mode):**
    * **Location:** Orchestrated by the `uiTask` created in `main.cpp`.
    * **Responsibility:** To run the full, feature-rich user interface for the main application.
    * **Architecture:** This is the full, multi-threaded UI engine. It consists of:
        * A dedicated `uiTask` pinned to **Core 1** to guarantee responsiveness.
        * The `InputManager` cabinet, which uses an ISR for the encoder and provides a simple polling interface.
        * The `StateManager` cabinet, which owns all screen objects and manages transitions.
        * The `UIManager` cabinet, which acts as the central rendering engine.
        * A collection of reusable **UI Blocks** (`MenuBlock`, `ButtonBlock`) located in `src/ui/blocks/`.
        * A collection of **Screens** (`MainMenuScreen`, etc.) located in `src/ui/screens/`.

### 3.2. Core UI Philosophy: Block-Based Assembly

The `pBIOS` and `Main` UI engines are both built on the principle that **screens do not draw themselves**. A screen's only responsibility is to act as a state machine. To render, it populates a shared data structure (`UIRenderProps`) that describes which reusable **UI Blocks** to display. The `UIManager` (in Normal mode) or the main loop (in pBios mode) is the sole rendering engine that interprets this data structure and calls the appropriate drawing functions for each block.

### 3.3. Input Handling

To ensure a "silky-smooth" and responsive user experience, all physical user input is handled by the `InputManager` cabinet. This cabinet uses a hardware interrupt for the rotary encoder to guarantee no rotation is ever missed. The main UI loop (either in `uiTask` or the `pBios` loop) calls `inputManager.update()` on every frame to get the latest debounced button states and the processed encoder "velocity engine" output.

## 4. Adopted Refinements (Critiques)

1.  **Unified `ConfigManager`:** A new cabinet will be created to provide a single interface for all configuration data.
2.  **Centralized Fault Handling:** A global `FaultHandler` will be implemented to centralize error logging and reporting.
3.  **Commitment to Unit Testing:** The modular "Cabinet" architecture will be leveraged by using PlatformIO's unit testing framework to test each module in isolation.

## 5. We will integrate testing directly into our development process.

1. **Develop a Cabinet First:** Before adding a new manager to main.cpp or creating an RTOS task for it, we first build the cabinet's class structure (.h and .cpp files).

2. **Write Tests for the Cabinet:** We then immediately create a corresponding test folder in the test directory. We write a series of tests to validate all of its core logic.

3. **Integrate Only After Passing:** Only after the cabinet passes all of its unit tests do we integrate it into the main firmware and its RTOS tasks.