# SpHEC Meter - Architectural Blueprint v2.0.0

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

## 2. System Cabinets & Engines

### 2.1. Filter Engine

* **Design:** A sequential, two-stage filter pipeline processes all analog sensor data. `Raw Data` -> `Filter 1 (HF)` -> `Filter 2 (LF)` -> `Calibrated Output`.
* **Filter 1 (High-Frequency):** A Median filter followed by a PI filter, designed to reject short-term spikes (<1s).
* **Filter 2 (Low-Frequency):** A Median filter followed by a PI filter, designed to smooth slower signal drift (<2min).
* **Tuning:** Filter tuning (`Settle Threshold`, `Lock Smoothing`, etc.) is performed **exclusively** in the **Diagnostics Mode** UI. In Normal Mode, the filters run "headlessly" using parameters loaded from internal flash.
* **Storage:** Filter parameters are stored in the ESP32's internal flash (NVS) via the `ConfigManager` for fast boot-time access.

### 2.2. Calibration Engine ("Smart Calibration")

* **Purpose:** To create a precise mathematical model translating filtered voltage into a scientific measurement (pH/EC), while also tracking probe health.
* [cite_start]**Model:** Uses a **quadratic function (y = ax² + bx + c)** for a more accurate curve of best fit over 3 calibration points[cite: 1, 93].
* **KPIs:** The engine calculates several Key Performance Indicators:
    * [cite_start]**Calibration Quality Score %:** Grades a new calibration's reliability based on goodness-of-fit (R-squared) and a slope sanity check[cite: 1, 97].
    * [cite_start]**Sensor Drift %:** Measures long-term probe aging by comparing new and previous calibration curves[cite: 1, 108].
    * [cite_start]**Calibration Health %:** Measures short-term drift via a 1-point voltage check[cite: 1, 115].
* **Storage:** The rich calibration models are saved to the SD card via the `StorageEngine`.

### 2.3. Power Monitor ("Intelligent Power Monitor")

* **Purpose:** To provide an accurate, KPI-driven view of battery state and health.
* [cite_start]**Hardware:** Uses an INA219 for bidirectional current sensing and is modeled on the Samsung INR18650-30Q battery[cite: 164, 299].
* **Hybrid Model:**
    * [cite_start]**Coulomb Counting:** Precisely tracks energy (Watt-hours) flowing into and out of the battery[cite: 164, 185].
    * [cite_start]**Voltage Reconciliation:** Uses Open-Circuit Voltage (OCV) to correct for drift after power-off events[cite: 164, 186].
    * [cite_start]**Stateful Learning:** An automated, long-term **SOH Recalibration Cycle** measures the battery's true capacity from a full charge to a low-level discharge, making the SOH value progressively more accurate[cite: 164, 227].
* [cite_start]**Storage:** The power monitor's state (SOH, cycles, learning cycle progress) is persisted to `battery_state.json` on the SD card via the `StorageEngine`[cite: 164, 239].

### 2.4. Storage, Backup, and "Limp Mode"

* **`StorageEngine`:** The **sole, mandatory interface** for all persistent configuration data operations (saving/loading models, state, etc.).
* **Atomic Writes:** To prevent data corruption from power loss, the engine uses an atomic write procedure: `write to .tmp` -> `delete .bak` -> `rename .json to .bak` -> `rename .tmp to .json`.
* **"Limp Mode":** To handle physical SD card failure, the storage layer is abstracted via an `I_StorageProvider` interface.
    * `SdCardProvider`: The default provider, interacts with the local SD card.
    * `RemoteStorageProvider`: A fallback provider that redirects all file operations over the wireless link to the connected Android app, allowing the device to function without a local SD card.

### 2.5. UI & Status System

* **`UIEngine` (Atelier):** A cabinet of modular, reusable UI components (menus, graphs, progress bars, wizards) that enforce a standard theme across the application.
* **`StatusIndicatorController`:** A central cabinet that manages the device's overall state. [cite_start]It evaluates active events based on a strict priority hierarchy to control the physical LEDs and the on-screen "State Stack"[cite: 1309, 1351].
* [cite_start]**System Tray:** A non-persistent status bar on OLEDs #1 & #2 showing the health of background components (probes, power busses, SD card)[cite: 1309, 1315].
* [cite_start]**State Stack:** A persistent, prioritized icon stack on OLED #3 that provides visual context for the physical Status LED's behavior[cite: 1309, 1342].

### 2.6. Connectivity

* **`ConnectivityManager`:** A cabinet that manages the connection state according to the hierarchy: **Wi-Fi Station (STA)** -> **BLE** -> **Wi-Fi Access Point (AP)**.
* **API Layer:** The device exposes a remote control API over HTTP (for Wi-Fi) and BLE GATT services (for Bluetooth) to allow full operation via an Android app.

## 3. Adopted Refinements (Critiques)

1.  **Unified `ConfigManager`:** A new cabinet will be created to provide a single interface for all configuration data. It will internally decide whether to fetch data from the fast internal flash (for filter params) or the SD card via `StorageEngine` (for large models).
2.  **Centralized Fault Handling:** A global `FaultHandler` will be implemented to centralize error logging and reporting, ensuring a consistent system-wide response to critical failures.
3.  **Commitment to Unit Testing:** The modular "Cabinet" architecture will be leveraged by using PlatformIO's unit testing framework to test each module in isolation, ensuring reliability before integration.

## 4. We will integrate testing directly into our development process.

1. **Develop a Cabinet First:** Before adding a new manager to main.cpp or creating an RTOS task for it, we first build the cabinet's class structure (.h and .cpp files).

2. **Write Tests for the Cabinet:** We then immediately create a corresponding test folder in the test directory. We write a series of tests to validate all of its core logic. `exp:` For the CalibrationManager, we would write tests to prove it can:

 # a. Correctly calculate the quadratic coefficients from 3 ideal data points.

 # b. Correctly calculate the "Calibration Quality Score".

 # c. Return an error if given invalid data.

3. **Integrate Only After Passing:** Only after the cabinet passes all of its unit tests do we integrate it into the main firmware and its RTOS tasks.

This "test-driven" approach is a best practice that prevents complex bugs by ensuring each individual component of our system is proven to be reliable before we connect them together.