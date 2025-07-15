

## **Project Manifest: SpHEC Meter**

**Version:** 4.0
**Last Updated:** July 5, 2025

This document is the **single source of truth** for the SpHEC Meter v9.0 project. It supersedes all previous manifests and design documents. All development must adhere strictly to the rules and specifications outlined herein.

### 1. The Golden Rules (Non-Negotiable)

* **Firmware is the Source of Truth:** All core logic, state management, and functionality resides on the ESP32 firmware. The companion app is only a UI mirror.
* **Verify Before Coding:** Before writing any code, you must read the relevant source file(s) to understand their established APIs and architecture.
* **Prove Your Work:** Demonstrate understanding by quoting the exact, relevant class definitions or function signatures from the project files before producing new code.
* **No Assumptions:** All reasoning must be based exclusively on the provided project source files. Do not use general programming knowledge that contradicts the project's established "ground truth."
* **Stop if Blocked:** If a file is missing or a tool fails, all work must stop. Report the failure and request the specific information needed to proceed.

---

### 2. Hardware Truths & Constraints

This section lists the non-negotiable hardware realities that all software must respect.

* **Processors & Framework:**
    * **Platform:** Espressif 32
    * **Board:** esp32dev
    * **Framework:** arduino

* **Bus Configuration:**
    * **I2C Pins:** `SDA` is on pin `21`, `SCL` is on pin `22`.
    * **SPI Pins (VSPI):** `MOSI` is on pin `23`, `MISO` is on pin `19`, `SCK` is on pin `18`.
    * **1-Wire Bus:** The 1-Wire bus is on pin `15`.
    * **DHT Sensor:** The DHT11 sensor is on pin `13`.

* **I2C Device Topology:**
    * The **INA219 Power Monitor** sits directly on the primary I2C bus at address `0x40`.
    * All other I2C devices (**RTC, OLEDs**) are behind a **TCA9548A Multiplexer** at address `0x70`.
    * The **RTC** is on channel `0` of the multiplexer.
    * **Software Mandate:** All I2C communication with the RTC or OLEDs *must* first select the correct channel via the `TCA9548_Driver`.

* **SPI Bus & Devices:**
    * **Fundamental Instability:** The SPI bus is considered untrustworthy.
    * **Software Mandate:** All SPI transactions must be protected by a mutex. Drivers must be stateless and must re-initialize bus settings for every transaction, explicitly managing all Chip Select (CS) lines.
    * **SD Card:** CS is on pin `5`.
    * **ADC1 (pH & 3.3V):** CS is on pin `4`.
    * **ADC2 (EC & 5V):** CS is on pin `2`.

* **ADC & Sensor Configuration:**
    * **ADC1:** Measures the **pH Probe** (Differential A0-A1) and the **3.3V Bus** (Single-Ended A2).
    * **ADC2:** Measures the **EC Probe** (Differential A0-A1) and the **5V Bus** (Single-Ended A2).
    * **Voltage Divider:** Both pH and EC sensor outputs are passed through a 10k+10k voltage divider before reaching the ADC.
    * **Software Mandate:** The `SensorProcessor` *must* multiply the raw ADC voltage reading by a factor of 2 to compensate.

* **LEDs & Logic:**
    * There is one top **bicolor (Red/Green) LED** and one bottom **single Green LED**.
    * The LEDs are **common anode**. `digitalWrite(pin, LOW)` turns the LED **ON**.
    * **Software Mandate:** All LED control logic must be encapsulated in the `LEDManager`. The "Amber" color is a software construct achieved by rapidly alternating the Red and Green LEDs.

* **Operational Modes:** The firmware operates in one of two modes,       
    determined at boot:
    * **Normal Mode:** Default mode. All RTOS tasks (`SensorTask`, `TelemetryTask`, etc.) are active. The UI boots to the `MainMenuScreen`.
    * **Diagnostics Mode (pBios):** Activated by holding the **Middle and Bottom buttons** during power-on. In this mode, only essential tasks (`UiTask`, `ConnectivityTask`, etc.) are created. Non-essential tasks like `SensorTask` and `TelemetryTask` are **not created**, ensuring a quiet system for sensitive measurements. The UI boots directly to the `DiagnosticsMenuScreen`.


---

### 3. Software Architecture: The "Cabinet" Model

The firmware is divided into four distinct layers based on the "Cabinet" philosophy. This enforces a one-way flow of dependencies, ensuring stability and modularity.

* **Guiding Principles:**
    * **Isolation:** Cabinets are black boxes that hide their internal complexity and expose a clean, simple public interface.
    * **Single Responsibility:** Each cabinet has one, and only one, job.
    * **Strict Layering:** Components can only interact with components in the layers directly below them.
    * **Data-Driven UI:** The Presentation Layer is strictly for displaying data and contains no business logic.

* **Modular Boot Sequence:** The `setup()` function in `main.cpp` delegates initialization to a sequence of functions in the `src/boot/` directory. This sequence is now **mode-aware**.

* **RTOS Design - Conditional Task Creation:** The `init_tasks` function now checks the global `g_boot_mode` variable. The full suite of tasks is only created in `NORMAL` mode. In `DIAGNOSTICS` mode, a minimal set of tasks is created to preserve system resources and ensure measurement integrity.

* **UI Architecture: Block-Based Assembly**: The user interface is built on a declarative model where screens assemble reusable UI Blocks (`MenuBlock`, `GraphBlock`, `ProgressBarBlock`).


---

### 4. The `helpers/` Directory: Shared Tooling

This is not a layer, but a **critical shared library** of complex, reusable code available to all layers, primarily the Manager Layer. It encapsulates mathematical and algorithmic complexity, keeping the managers focused on state and orchestration.

* **File Structure (`src/helpers/`):**
    * `filters/ExponentialFilter.h/.cpp`
    * `filters/KalmanFilter.h/.cpp`
    * `calibration/QuadraticModel.h/.cpp`
    * `calibration/CalibrationUtils.h`
* **Role:**
    * **`helpers/filters/`**: This directory is the home for rich and complex filtering algorithms. It contains various standalone filter classes (e.g., `ExponentialFilter`, `KalmanFilter`) that can be instantiated and used by any manager that needs them, primarily the `SensorProcessor`.
    * **`helpers/calibration/`**: This directory houses reusable helper functions and classes for the calibration process, such as a generic `QuadraticModel` class that can be configured with specific coefficients for pH, EC, etc. This prevents code duplication.

---

### 5. Legacy-to-New Cabinet Mapping

This table serves as the official translation map from the legacy project to the new v9.0 architecture.

| Legacy Component / File | New Cabinet (Layer & Name) | Notes |
| :--- | :--- | :--- |
| Smart\_EC\_pH\_Meter.ino | Entry Point (`main.cpp`) | Contains minimal `setup()`/`loop()` delegating to Application Layer tasks. |
| INA219.h/.cpp | **HAL** `INA219_Driver` | Unchanged. Direct hardware communication. |
| ADS1118.h/.cpp | **HAL** `ADS1118_Driver` | Unchanged. Direct hardware communication. |
| TCA9548A.h/.cpp | **HAL** `TCA9548_Driver` | Unchanged. Direct hardware communication. |
| PCF8563.h/.cpp | **HAL** `PCF8563_Driver` | New HAL cabinet for the RTC chip. |
| OneWireBus.h/.cpp | **HAL** `OneWire_Driver` | New HAL cabinet for the 1-Wire bus. |
| DHT.h/.cpp | **HAL** `DHT_Driver` | New HAL cabinet for the DHT11 sensor. |
| Power handling logic | **Manager** `PowerManager` | Refactored to manage battery state-of-charge and health. |
| EEPROM / SD logic | **Manager** `StorageManager` | Centralizes all interaction with the SD card. |
| WiFi connection logic | **Manager** `WifiManager` | Provides a clean API for Wi-Fi operations (connect, disconnect, mode change). |
| BLE service logic | **Manager** `BleManager` | Encapsulates all BLE communication and services. |
| MQTT service logic | **Manager** `MqttManager` | Manages connection and publishing to the MQTT broker. |
| RTC interaction logic | **Manager** `RtcManager` | Wraps the `PCF8563_Driver` and manages time synchronization. |
| Encoder/Button logic | **Manager** `EncoderManager`, `ButtonManager` | Manages physical user inputs. |
| Status LED logic | **Manager** `LEDManager` | Provides a stateless API (`setTopLedColor`, etc.) driven by the Application Layer. |
| pH, EC, sensor pipelines| **Manager** `SensorProcessor` | Core data processing: gets raw data from HAL, applies calibration/filters. |
| Main UI loop | **Application** `UiTask` | Determines active screen and passes data to `UIManager` for rendering. |
| Web Server logic | **Application** `WebService` | Manages API endpoints and WebSocket server. |
| System state machine | **Application** `SystemStateController` | Manages high-level state transitions (NORMAL, pBIOS, etc.). |
| Telemetry publishing | **Application** `TelemetryTask` | Periodically triggers data serialization and publishing. |
| Stateful indicator logic | **Application** `StatusIndicatorController`| The "brain" for LEDs; observes system state and commands the `LEDManager`. |
| `filters/*.h`, `calibration/*.h` | `helpers/` Directory | Moved and refactored into the shared helper library. |

---

### 6. Feature & API Requirements

This is the definitive list of required functionality.

* **Connectivity:**
    * **BLE:** Must provide a BLE service for configuration and telemetry.
        * Service UUID: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
        * Telemetry Characteristic UUID: `beb5483e-36e1-4688-b7f5-ea07361b26a8`
    * **Wi-Fi:** Must support both **Access Point (AP)** and **Station (STA)** modes.
    * **MQTT:** Must connect to `test.mosquitto.org:1883` and publish telemetry to `sphec/meter/telemetry`.
    * **Web Server:** An `ESPAsyncWebServer` must be implemented.
* **Web API & Data Handling:**
    * **Critical Memory Issue:** The design **must resolve** the system crashes caused by large `JsonDocument` allocations in request handlers.
    * **Requirement:** Decouple JSON serialization from the `WebService`. A dedicated `TelemetrySerializer` cabinet will generate a single, reusable telemetry string to be used by the web, BLE, and MQTT services.
    * **Configuration API (`/api/v1/config/set`):** A lean, key-value based endpoint for sending Wi-Fi/MQTT credentials via URL-encoded pairs.
    * **WebSocket:** A WebSocket server for high-speed, real-time telemetry streaming.
    * **SD Card File Transfer:** API endpoints for backup and restore of configuration files.
* **Required Libraries (`lib_deps`):**
    * `bblanchon/ArduinoJson@^6.19.4`
    * `adafruit/Adafruit INA219`
    * `adafruit/Adafruit GFX Library`
    * `adafruit/Adafruit SSD1306`
    * `adafruit/RTClib`
    * `me-no-dev/ESPAsyncWebServer@^1.2.4`
    * `me-no-dev/AsyncTCP@^1.1.1`
    * `marvinroger/AsyncMqttClient@^0.9.0`

---

### 7. Phased Development Plan

* **Phase 1: Architectural Refactoring & API Redesign (Current Phase)**
    * **Goal:** Fix the critical memory allocation failure.
    * **Tasks:**
        1.  Create the `TelemetrySerializer` cabinet.
        2.  Refactor `WebService`, `BleManager`, and `MqttManager` to consume the pre-serialized telemetry string.
        3.  Implement the lean `/api/v1/config/set` endpoint.
* **Phase 2: Core Firmware Feature Completion**
    * **Goal:** Build out the remaining firmware features on the new, stable architecture.
    * **Tasks:**
        1.  Implement the WebSocket server.
        2.  Implement Wi-Fi mode selection (AP vs. STA).
        3.  Build the file transfer API for SD card backup and restore.
* **Phase 3: Companion App Implementation (On Hold)**
    * **Trigger:** Begins *only* after the Phase 2 firmware is stable and feature-complete.
    * **Goal:** Build the Android app as a polished UI mirror of the firmware's robust API.

---

### 8. Layered Architecture Breakdown

This section provides a consolidated view of the entire software architecture, mapping responsibilities, files, and features to each layer.

#### **Layer 1: HAL (Hardware Abstraction Layer)**

* **Responsibilities:** Direct, stateless communication with physical hardware components.
* **File Structure (`src/hal/`):**
    * `INA219_Driver.h/.cpp`
    * `ADS1118_Driver.h/.cpp`
    * `TCA9548_Driver.h/.cpp`
    * `PCF8563_Driver.h/.cpp`
    * `OneWire_Driver.h/.cpp`
    * `DHT_Driver.h/.cpp`

#### **Layer 2: Manager Layer**

* **Responsibilities:** The "brains" of the operation. Contains core logic, state management, and services.
* **File Structure (`src/managers/`):**
    * `PowerManager.h/.cpp`
    * `StorageManager.h/.cpp`
    * `WifiManager.h/.cpp`
    * `BleManager.h/.cpp`
    * `MqttManager.h/.cpp`
    * `RtcManager.h/.cpp`
    * `EncoderManager.h/.cpp`
    * `ButtonManager.h/.cpp`
    * `LEDManager.h/.cpp`
    * `SensorProcessor.h/.cpp`
* **Interaction with `helpers/`:** The **`SensorProcessor`** is the primary consumer of the `helpers/` library, instantiating and using classes from `helpers/filters/` and `helpers/calibration/` to turn raw sensor data into final, stable values.

#### **Layer 3: Application Layer**

* **Responsibilities:** Orchestrates the managers to create a functioning application. Contains high-level logic, RTOS tasks, and the system state machine.
* **File Structure (`src/app/`):**
    * `UiTask.h/.cpp`
    * `WebService.h/.cpp`
    * `SystemStateController.h/.cpp`
    * `TelemetryTask.h/.cpp`
    * `StatusIndicatorController.h/.cpp`

#### **Layer 4: Presentation Layer (UI)**

* **Responsibilities:** Displays data and presents the UI to the user. Contains no business logic. Defines the structure and content of UI views based on the **Four-Core UI Architecture**.
* **Key Document**: All UI development must adhere to the principles and patterns outlined in the **`UIManifest.md`**.
* **File Structure (`src/presentation/`):**
    * `UIManager.h/.cpp` (The "Canvas" Renderer)
    * `DisplayManager.h/.cpp` (The Display HAL)
    * `common/` (Shared UI types and props)
    * `resources/` (Bitmap icon assets)
    * `screens/` (Individual, modular screen logic)
    * `wizards/` (Reusable wizard and step classes)















## **Project Manifest: SpHEC Meter**

**Version:** 1.5.1
**Last Updated:** July 12, 2025

This document is the **single source of truth** for the SpHEC Meter project. It supersedes all previous manifests and design documents. All development must adhere strictly to the rules and specifications outlined herein.

### 1. The Golden Rules (Non-Negotiable)

* **Firmware is the Source of Truth:** All core logic, state management, and functionality resides on the ESP32 firmware. The companion app is only a UI mirror.
* **Verify Before Coding:** Before writing any code, you must read the relevant source file(s) to understand their established APIs and architecture.
* **Prove Your Work:** Demonstrate understanding by quoting the exact, relevant class definitions or function signatures from the project files before producing new code.
* **No Assumptions:** All reasoning must be based exclusively on the provided project source files. Do not use general programming knowledge that contradicts the project's established "ground truth."
* **Stop if Blocked:** If a file is missing or a tool fails, all work must stop. Report the failure and request the specific information needed to proceed.

---

### 2. Hardware Truths & Constraints

This section lists the non-negotiable hardware realities that all software must respect.

* **Processors & Framework:**
    * **Platform:** Espressif 32
    * **Board:** esp32dev
    * **Framework:** arduino
* **Bus Configuration:**
    * **I2C Pins:** `SDA` is on pin `21`, `SCL` is on pin `22`.
    * **SPI Pins (VSPI):** `MOSI` is on pin `23`, `MISO` is on pin `19`, `SCK` is on pin `18`.
* **I2C Device Topology & Initialization Sensitivity:**
    * The **INA219 Power Monitor** sits directly on the primary I2C bus at address `0x40`.
    * All other I2C devices (**RTC, OLEDs**) are behind a **TCA9548A Multiplexer** at address `0x70`.
    * The **RTC** is on channel `0` of the multiplexer.
    * **Software Mandate:** The I2C bus has proven to be extremely sensitive during the initial boot sequence. All I2C device initializations **must** be centralized. The working sequence from the legacy firmware (`DisplayManager` then `RtcManager`) must be precisely replicated to ensure bus stability.

* **SPI Bus & Devices:**
    * **Fundamental Instability:** The SPI bus is considered untrustworthy. All SPI transactions must be protected by a mutex.
    * **SD Card:** CS is on pin `5`.
    * **ADC1 (pH & 3.3V):** CS is on pin `4`.
    * **ADC2 (EC & 5V):** CS is on pin `2`.

---

### 3. Software Architecture: The "Cabinet" & Modular Boot Model

The firmware is divided into four distinct layers based on the "Cabinet" philosophy. This enforces a one-way flow of dependencies, ensuring stability and modularity.

The `setup()` function has been refactored into a **modular boot sequence**. All initialization logic is delegated to functions within the `src/boot/` directory to improve clarity and maintainability. This sequence is:
1.  `init_globals()`
2.  `init_i2c_devices()`
3.  `init_hals()`
4.  `init_managers()`
5.  `run_post()` (Power-On Self-Test)
6.  `init_tasks()`

---

### 4. Current Status & Blocking Issue

The project's architecture has been significantly improved by refactoring the boot sequence into a modular, multi-stage process. However, the project is currently **blocked** by a persistent runtime crash.

* **Symptom:** A `NULL TX buffer pointer` error occurs during the `DisplayManager::begin()` call, preventing the OLED screens from initializing.
* **Known Facts:**
    * The hardware is confirmed to be functional, as the legacy firmware works correctly.
    * The crash is not a compile-time error but a runtime I2C bus corruption.
* **Current Strategy:** The focus is on a meticulous, step-by-step diagnostic process to isolate the exact line of code that is corrupting the I2C bus state during initialization. All further development is on hold until this critical boot issue is resolved.

---

### 5. Feature & API Requirements

*This section remains unchanged from the previous manifest version.*

* **Connectivity:**
    * **BLE:** Must provide a BLE service for configuration and telemetry.
    * **Wi-Fi:** Must support both **Access Point (AP)** and **Station (STA)** modes.
    * **MQTT:** Must connect to `test.mosquitto.org:1883` and publish telemetry.
* **Web API & Data Handling:**
    * A dedicated `TelemetrySerializer` must be used to generate a single, reusable telemetry string.
    * A lean `/api/v1/config/set` endpoint is required.
    * A WebSocket server for real-time telemetry is required.









    # **Project Manifest: SpHEC Meter**

**Version:** 1.5.0
**Last Updated:** July 14, 2025

This document is the **single source of truth** for the SpHEC Meter v1.5.0 project. It supersedes all previous manifests and design documents. All development must adhere strictly to the rules and specifications outlined herein.

### 1. The Golden Rules (Non-Negotiable)

* **Firmware is the Source of Truth:** All core logic, state management, and functionality resides on the ESP32 firmware. The companion app is only a UI mirror.
* **Verify Before Coding:** Before writing any code, you must read the relevant source file(s) to understand their established APIs and architecture.
* **Prove Your Work:** Demonstrate understanding by quoting the exact, relevant class definitions or function signatures from the project files before producing new code.
* **No Assumptions:** All reasoning must be based exclusively on the provided project source files. Do not use general programming knowledge that contradicts the project's established "ground truth."

---

### 2. Software Architecture: The "Cabinet" Model

The firmware is divided into distinct layers based on the "Cabinet" philosophy. This enforces a one-way flow of dependencies, ensuring stability and modularity.

* **UI Architecture: Block-Based Assembly**: The user interface is not monolithic. Screens are simple "assemblers" that do not contain drawing logic. Instead, they select from a library of reusable UI **Blocks** (e.g., `MenuBlock`, `GraphBlock`) and provide them with data. The `UIManager` is responsible for rendering these blocks. This is detailed in `UIManifest.md`.
* **Input Handling: Event-Driven**: User input from the rotary encoder is handled by a high-priority, dedicated RTOS task (`EncoderTask`) that processes hardware interrupts via a queue. This ensures a "silky smooth," responsive feel that is completely decoupled from the UI rendering loop.
* **System Modes: Focused Analysis**: For computationally intensive operations like noise analysis, the system will enter a "Focused Analysis Mode". This involves temporarily suspending non-essential background tasks (like Telemetry and Connectivity) to dedicate maximum system resources to the analysis, ensuring data integrity.

---

### 3. Phased Development Plan

* **Phase 1: Architectural Refactoring & Stabilization (COMPLETE)**
    * **Status:** All critical boot-up issues (I2C, RTOS primitives), memory leaks, and hardware interface bugs have been resolved. The system is stable. The core UI interaction model (encoder, buttons, screen navigation) is functional and smooth.

* **Phase 2: Core Feature Completion (IN PROGRESS)**
    * **Goal:** Build out the core functionality of the device on the new, stable architecture.
    * **Current Task:** Implementing the **Noise Analysis Engine**. This includes the backend `NoiseAnalysisManager` and its helpers (`StatisticalAnalyzer`, `FftAnalyzer`, `AutoTuner`) and connecting them to the user interface.
    * **Next Steps:** Implement the full calibration workflow and the remaining menu screens.

---










# **Project Manifest: SpHEC Meter**

**Version:** 1.6.2
**Last Updated:** July 15, 2025

This document is the **single source of truth** for the SpHEC Meter project. It supersedes all previous manifests and design documents. All development must adhere strictly to the rules and specifications outlined herein.

### 1. The Golden Rules (Non-Negotiable)

* **Firmware is the Source of Truth:** All core logic resides on the ESP32 firmware. The companion app is only a UI mirror.
* **Verify Before Coding:** Read relevant source files to understand established APIs and architecture.
* **Prove Your Work:** Demonstrate understanding by quoting exact class definitions or function signatures.
* **No Assumptions:** All reasoning must be based exclusively on the provided project source files.
* **Stop if Blocked:** If a file is missing or a tool fails, stop and report the failure.

---

### 2. Hardware & Operational Truths

This section lists the non-negotiable hardware and operational realities that all software must respect.

* **(Unchanged Hardware Sections: I2C, SPI, ADC, etc.)**
* **Operational Modes:** The firmware operates in one of two modes, determined at boot:
    * **Normal Mode:** Default mode. All RTOS tasks (`SensorTask`, `TelemetryTask`, etc.) are active. The UI boots to the `MainMenuScreen`.
    * **Diagnostics Mode (pBios):** Activated by holding the **Middle and Bottom buttons** during power-on. In this mode, only essential tasks (`UiTask`, `ConnectivityTask`, etc.) are created. Non-essential tasks like `SensorTask` and `TelemetryTask` are **not created**, ensuring a quiet system for sensitive measurements. The UI boots directly to the `DiagnosticsMenuScreen`.

---

### 3. Software Architecture: The "Cabinet" Model & Modular Boot

The firmware is divided into distinct layers based on the "Cabinet" philosophy.

* **Modular Boot Sequence:** The `setup()` function in `main.cpp` delegates initialization to a sequence of functions in the `src/boot/` directory. This sequence is now **mode-aware**.
* **RTOS Design - Conditional Task Creation:** The `init_tasks` function now checks the global `g_boot_mode` variable. The full suite of tasks is only created in `NORMAL` mode. In `DIAGNOSTICS` mode, a minimal set of tasks is created to preserve system resources and ensure measurement integrity.
* **UI Architecture: Block-Based Assembly**: The user interface is built on a declarative model where screens assemble reusable UI Blocks (`MenuBlock`, `GraphBlock`, `ProgressBarBlock`).

---









