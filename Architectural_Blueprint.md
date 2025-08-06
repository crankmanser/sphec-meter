
# SpHEC Meter - Architectural Blueprint v2.11.19

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

### 2.1. Filter Engine: A Specialised Two-Stage Pipeline

* **Design:** A sequential, two-stage filter pipeline processes all analog sensor data: `Raw Data` -> `Filter 1 (HF)` -> `Filter 2 (LF)` -> `Calibrated Output`. The two stages are specialized for different noise domains.
* **Filter 1 (High-Frequency "Spike Scraper"):** This stage is specialized for rejecting fast, sharp noise spikes (<1s). It uses a small median window and gentle PI smoothing parameters to "clip off" jagged peaks without distorting the underlying signal.
* **Filter 2 (Low-Frequency "Smoothing Squeegee"):** This stage is specialized for smoothing long-term signal drift (<2min). It uses a large median window and aggressive PI smoothing parameters to heavily average the signal from the HF stage, flattening the slow wave and finding the true, stable center.
* **Tuning KPIs:** The filter provides real-time KPIs for data-driven tuning:
    * **F_std (Filtered Standard Deviation):** A quantitative measure of the filtered signal's noise. The primary goal of tuning is to minimize this value.
    * **R_std (Raw Standard Deviation):** A baseline measure of the hardware's inherent noise level.
    * **Stab % (Stability):** A real-time percentage (0-100%) indicating noise reduction efficiency.

### 2.2. Calibration Engine ("Smart Calibration")

* **Purpose:** To create a precise mathematical model translating filtered voltage into a scientific measurement (pH/EC), while also tracking probe health. The calibration is performed on the **final, stable output of the LF filter**, ensuring any attenuation from the filter pipeline is automatically accounted for in the model.
* **Model:** Uses a **quadratic function (y = ax² + bx + c)** for a more accurate curve of best fit over 3 calibration points.
* **Hybrid Procedure:** A two-stage process is required for modules like the PH-4502C:
    1.  **Hardware Calibration:** A one-time physical adjustment of the sensor module's offset potentiometer, guided by a dedicated wizard in the main application UI.
    2.  **Software Calibration:** The standard 3-point calibration wizard which builds the quadratic model.
* **KPIs (Read-Only Diagnostics):** The engine calculates several Key Performance Indicators which are presented to the user as a read-only "report card" on sensor health. The only way to improve these KPIs is to perform a new, high-quality calibration.
    * **Calibration Quality Score %:** Grades a new calibration's reliability based on goodness-of-fit (R-squared) and a slope sanity check.
    * **Sensor Drift %:** Measures long-term probe aging by comparing new and previous calibration curves.
    * **Zero-Point Drift (mV):** Measures the long-term drift of the hardware's physical offset.

### 2.3. Power Monitor ("Intelligent Power Monitor")
* **(Design Unchanged)**

### 2.4. Storage, Backup, and "Limp Mode"
* **Dual-Save Strategy:** To ensure robustness, the system uses a dual-save approach.
    * **ESP32 NVS (Internal Flash):** Stores the essential operational data: the latest **Filter Setpoints** and the current **Calibration Model**. This allows the device to boot quickly and function perfectly even if the SD card is missing ("Limp Mode").
    * **SD Card:** Stores detailed **Tuning Log Files** and configuration backups for the companion Android suite. Each log contains a snapshot of the noise analysis, the auto-tuner's proposed values, the user's final values, and the resulting KPIs.

## 3. User Interface Architecture

### 3.1. Dual-Boot System
* **(Design Unchanged)**

### 3.2. The pBIOS UI Engine & "Guided Tuning" Workflow

* **Architecture:** A dedicated, dual-core RTOS system that provides a powerful interface for diagnostics and tuning. It is designed to be lean and stable, initializing only the hardware essential for its diagnostic tasks.

* **"Guided Tuning" Workflow:** The "Live Filter Tuning" feature implements a sophisticated but stable guided tuning process:
    1.  **Entry Point:** When the user enters the `LiveFilterTuningScreen`, the `pBiosDataTask` triggers the `GuidedTuningEngine`.
    2.  **Heuristic Analysis:** The engine uses a data-driven, heuristic algorithm (combining Statistical and FFT analysis) to measure the noise characteristics of the signal.
    3.  **Propose Baseline:** Based on the analysis, it calculates a complete set of recommended starting parameters for both the specialized HF and LF filter stages.
    4.  **User Fine-Tuning:** These parameters are automatically loaded, and the user is presented with the live graphs, which are already running with this intelligent baseline. The user then performs the final fine-tuning and can permanently save their setpoints.
    * **Implementation Note:** The `GuidedTuningEngine`'s analysis and parameter derivation logic is computationally and memory-intensive. Initial implementations have led to critical heap corruption and stack overflow errors on the ESP32. The final implementation **must** be carefully designed to be RTOS-friendly. It must avoid large local variable allocations on the task stack and minimize or eliminate rapid memory allocation/deallocation patterns on the heap to ensure system stability.


* **Finalized pBIOS Menu Structure:** The pBIOS provides a comprehensive suite of engineering and maintenance tools, organized into a clear menu structure.
    * **`Noise Analysis`**: A tool for performing a high-speed statistical analysis of a selected signal, providing key metrics like Mean, Min/Max, and Standard Deviation.
    * **`NA Drift Trending`**: A tool for performing a long-duration FFT (Fast Fourier Transform) analysis to identify low-frequency signal drift and interference.
    * **`Live Filter Tuning`**: The main interface for running the Guided Tuning algorithm and performing manual fine-tuning of the two-stage filter pipeline.
    * **`Maintenance` (Sub-Menu)**:
        * **`Probe Analysis`**: A pBIOS-centric "report card" for a probe, displaying its live signal integrity (`R_std`) and its current "Filter Creep" (the saved filter parameters), which serve as an indirect measure of probe aging.
        * **`New Probe`**: A utility to reset the filter configuration for a specific probe to factory defaults.
        * **`Hardware Self-Test`**: A lean, pBIOS-safe diagnostic routine that verifies the function of the SD Card, all three OLEDs, both ADCs, and the temperature probe.
        * **`Live ADC Voltmeter`**: A utility to display the live, raw millivolt reading from any ADC channel for low-level hardware debugging.
        * **`pBIOS Snapshot`**: A function to save a complete diagnostic file to the SD card, containing current filter settings and the results of a fresh noise analysis.
        * **`SD Card Formatter`**: A utility to format the SD card.
        * **`Android Suite`**: (Placeholder)
    * **`Shutdown` (Sub-Menu)**: A menu to ensure the safe persistence of settings before the user turns off the physical power switch. Provides options to `Save & Shutdown`, `Discard & Shutdown`, or `Restore Defaults & Shutdown`.


### 3.3. Core UI Philosophy: Block-Based Assembly

Both UI engines are built on the principle that **screens do not draw themselves**. A screen's role is to manage state and populate a `UIRenderProps` data structure. The central `UIManager` is the sole rendering engine that interprets this structure and calls the draw functions for reusable **UI Blocks** (`MenuBlock`, `GraphBlock`, `ButtonBlock`).


## 4. Android Suite Feedback Loop

The system is designed to support a long-term learning feedback loop with a companion Android app.
1.  The app will sync the detailed tuning logs and `pBIOS Snapshots` from the meter's SD card.
2.  It will perform off-device analysis on the "deviation data" (the difference between the algorithm's proposals and the user's final settings) and the "filter creep" data over time.
3.  Based on this analysis, it can generate an updated set of heuristic rules for the auto-tuner.
4.  This updated rule file is sent back to the meter and saved by the `ConfigManager`, making the device's "guided tuning" feature progressively smarter over time.


## 5. Adopted Refinements & Planned Architecture Enhancements

1.  **Unified `ConfigManager`:** A single cabinet provides a unified interface for all configuration data, abstracting the underlying storage medium (NVS vs. SD card).
2.  **Centralized Fault & Status Handling:** A global `FaultHandler` manages critical errors. This is supplemented by a `SystemStatus` manager to track and report non-fatal warnings (e.g., "SD Card missing," "Probe Disconnected"), allowing the system to operate in a degraded "Limp Mode" and provide richer feedback to the user.
3.  **Commitment to Unit Testing:** The modular "Cabinet" architecture is leveraged by using PlatformIO's unit testing framework to test each module in isolation.
4.  **External Diagnostic Jig:** Advanced hardware characterization (such as measuring the ADC's open-pin noise floor) will be performed using a simple, external diagnostic jig connected to a dedicated terminal. This preserves the electrical integrity of the main board and eliminates the need for complex and potentially noisy internal switching circuits.

## 6. We will integrate testing directly into our development process.

1.  **Develop a Cabinet First:** Before adding a new manager to main.cpp or creating an RTOS task for it, we first build the cabinet's class structure (.h and .cpp files).

2.  **Write Tests for the Cabinet:** We then immediately create a corresponding test folder in the test directory. We write a series of tests to validate all of its core logic.

3.  **Integrate Only After Passing:** Only after the cabinet passes all of its unit tests do we integrate it into the main firmware and its RTOS tasks.