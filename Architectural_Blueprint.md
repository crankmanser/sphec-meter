# SpHEC Meter - Architectural Blueprint v2.2.0

This document is the "genesis document" and official architectural summary for the SpHEC Meter firmware. It encapsulates all design decisions made during our initial planning phase and serves as the foundational context for all future development.

## 1. Core Philosophy & Architecture

The firmware is built on a **"Cabinet" philosophy**. Each major component (e.g., `PowerMonitor`, `CalibrationManager`) is a self-contained module with a clear responsibility, promoting separation of concerns.

### 1.1. FreeRTOS Task Structure

The system uses a dual-core FreeRTOS architecture to ensure UI responsiveness and operational stability.

| Task | Core | Priority | Purpose |
|---|---|---|---|
| `uiTask` / `pBiosUiTask` | 1 | 3 (High) | Manages all user input, screen state, and display updates. |
| `sensorTask` / `pBiosDataTask` | 0 | 2 (Norm) | Handles all data acquisition and processing. |
| `i2cTask`, `oneWireTask`, `sdTask` | 0 | 1 (Low) | Handle other peripheral communication. |


### 1.2. Inter-Task Communication

* **Asynchronous Request Queues:** Used for slow or blocking operations. The `sdTask` uses a queue to receive file requests, decoupling other tasks from slow SD card I/O.
* **Mutex-Protected Shared Data:** A central `GlobalDataModel` struct, protected by a FreeRTOS mutex, will hold the latest aggregated and processed sensor data for thread-safe access by any task. This prevents race conditions and is superior to critical sections for inter-task resource sharing.

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
* **Tuning KPIs:** The filter provides real-time KPIs for data-driven tuning:
    * **F_std (Filtered Standard Deviation):** A quantitative measure of the filtered signal's noise. The primary goal of tuning is to minimize this value.
    * **R_std (Raw Standard Deviation):** A baseline measure of the hardware's inherent noise level.
    * **Stab % (Stability):** A real-time percentage (0-100%) indicating noise reduction efficiency.

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
    * **Settling Time (s):** A new KPI that measures the time taken for a probe's signal to become stable after being activated, providing another metric for probe health.

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

The UI is built on a robust, modular architecture designed for stability and responsiveness. It utilizes a single, globally initialized `InputManager` and separates the application into two distinct boot modes.

### 3.1. Dual-Boot System

The firmware uses a simple, reliable method to select the operating mode on power-up, mirroring the stable legacy implementation.
* **Mode Selection:** The boot mode is determined by checking if the **middle ("Enter") button** is held down during the initial boot sequence.
* **Normal Boot:** If no button is pressed, the device proceeds to launch the main application.
* **pBIOS Boot:** If the button is held down, the device launches the pBIOS diagnostics suite.
* **Stability:** This approach eliminates the complexities of RTC memory and software reboots, guaranteeing a clean hardware state for each mode.

### 3.2. The Two UI Engines

1.  **The pBIOS UI Engine (Diagnostics Mode):**
    * **Architecture:** A dedicated, dual-core RTOS system (`pBiosUiTask` on Core 1, `pBiosDataTask` on Core 0) that is launched when pBIOS mode is selected. It provides a lightweight but powerful interface for all diagnostic functions.
    * **Live Filter Tuning UI:** The LFT workflow is a key feature:
        1.  The user first enters a `FilterSelectionScreen` to choose which sensor to tune.
        2.  They are then taken to the `LiveFilterTuningScreen`, which uses all three OLEDs:
            * **Top/Bottom OLEDs:** Display real-time graphs for the HF and LF filter stages.
            * **Middle OLED:** Displays a status area at the top showing the selected parameter's live value, with a menu list below for navigation.
        3.  Selecting a parameter takes the user to a dedicated `ParameterEditScreen` for a focused editing experience.

2.  **The Main UI Engine (Normal Mode):**
    * **Architecture:** The full, multi-threaded UI engine orchestrated by the main `uiTask` on Core 1. It manages the complete user experience for measurement, calibration, and settings.

### 3.3. Core UI Philosophy: Block-Based Assembly

Both UI engines are built on the principle that **screens do not draw themselves**. A screen's role is to manage state and populate a `UIRenderProps` data structure. The central `UIManager` is the sole rendering engine that interprets this structure and calls the draw functions for reusable **UI Blocks** (`MenuBlock`, `GraphBlock`, `ButtonBlock`).


## 4. Adopted Refinements & Planned Architecture Enhancements

1.  **Unified `ConfigManager`:** A new cabinet will be created to provide a single interface for all configuration data. It will abstract the underlying storage medium (NVS vs. SD card) from the rest of the application.
2.  **Centralized Fault & Status Handling:** A global `FaultHandler` will continue to manage critical errors. This will be supplemented by a `SystemStatus` manager to track and report non-fatal warnings (e.g., "SD Card missing"), allowing the system to operate in a degraded "Limp Mode" and provide richer feedback to the user.
3.  **Commitment to Unit Testing:** The modular "Cabinet" architecture will be leveraged by using PlatformIO's unit testing framework to test each module in isolation. We will continue to follow a "Test-Driven Development" approach where possible.

## 5. We will integrate testing directly into our development process.

1.  **Develop a Cabinet First:** Before adding a new manager to main.cpp or creating an RTOS task for it, we first build the cabinet's class structure (.h and .cpp files).

2.  **Write Tests for the Cabinet:** We then immediately create a corresponding test folder in the test directory. We write a series of tests to validate all of its core logic.

3.  **Integrate Only After Passing:** Only after the cabinet passes all of its unit tests do we integrate it into the main firmware and its RTOS tasks.