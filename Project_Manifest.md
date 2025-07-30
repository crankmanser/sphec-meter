# SpHEC Meter Project Manifest

This document tracks the development progress, current tasks, and future roadmap for the SpHEC Meter firmware.

## Changelog (What Was Done)

* **v2.9.4 (2025-07-29):**
    * **Status:** Complete.
    * **Milestone:** Successfully completed **Phase 1 of the UI implementation**. The foundational back-end for the user interface is now in place and stable.
    * **Feature:** Created the `InputManager`, `StateManager`, and `UIManager` cabinets, establishing the core plumbing for an event-driven, declarative UI.
    * **Feature:** Implemented the main `uiTask`, which orchestrates input handling, state management, and rendering on its own RTOS task pinned to Core 1.
    * **Fix:** Resolved a critical `Guru Meditation Error` (IllegalInstruction) by re-architecting the UI initialization sequence to be RTOS-safe. The `uiTask` is now responsible for setting its own initial state, resolving the race condition that caused the crash.
* **v2.9.3 (2025-07-28):**
    * **Status:** Planning Complete.
    * **Milestone:** Finalized a comprehensive, four-phase development plan for implementing the graphical user interface, building upon the stable back-end architecture. The plan includes a dual-boot pBios system, an event-driven input manager, and a declarative, block-based rendering engine.
* **v2.9.2 (2025-07-28):**
    * **Status:** Complete.
    * **Milestone:** The entire back-end sensor and calibration engine is now feature-complete, stable, and validated on hardware.
    * **Fix:** Resolved the persistent, erratic raw voltage readings by re-implementing the critical **"Priming Read"** timing requirement in the `AdcManager`, where two consecutive ADC reads are performed and the first is discarded to ensure a stable signal.
    * **Feature:** Added a comprehensive **Hardware Calibration Wizard** via the serial monitor to guide the user through the physical adjustment of the PH-4502C module's offset potentiometer.
    * **Feature:** Enhanced the `CalibrationManager`'s diagnostics by adding the **"Zero-Point Drift" KPI**, which tracks the hardware's neutral voltage offset over time to monitor long-term probe health.
    * **Enhancement:** Made all serial wizard interactions fully blocking to improve user experience.
* **v2.8.0 (2025-07-28):**
    * **Status:** Complete.
    * **Milestone:** All hardware sensors are now online and integrated into the firmware's data processing pipeline.
    * **Feature:** Created the `TempManager` cabinet and `oneWireTask` to bring the DS18B20 and DHT11 temperature sensors online.
    * **Feature:** Implemented a simple moving average filter in the `PowerMonitor` to smooth the noisy current readings from the INA219.
    * **Feature:** Enhanced the `AdcManager` to read from specific ADC channels, enabling the monitoring of the 3.3V and 5.0V system busses.
    * **Feature:** Added dedicated `FilterManager` instances for all new analog inputs.
    * **Integration:** Updated the `sensorTask` and `telemetryTask` to process and display data from every sensor on the board.
* **v2.5.0 (2025-07-27):**
    * **Status:** Complete.
    * **Milestone:** The back-end calibration engine is feature-complete and validated.
    * **Feature:** Implemented the full KPI calculation logic within the `CalibrationManager`, including "Calibration Quality Score %" (based on R-squared and slope analysis) and "Sensor Drift %" (based on integrated deviation).
    * **Unit Test:** Updated and passed the unit test for `CalibrationManager` to validate the correctness of the new KPI calculations.
    * **Integration:** Created a temporary "Serial Calibration Wizard" task to allow for a full 3-point calibration via the serial monitor, proving the entire workflow from data entry to model calculation and saving to the SD card.
* **v2.4.0 (2025-07-27):**
    * **Status:** Complete.
    * **Milestone:** Successfully implemented the core `CalibrationManager` cabinet and integrated it into the main data pipeline.
    * **Feature:** Created the `CalibrationManager` cabinet to perform quadratic regression, converting filtered voltages into scientific values (pH/EC).
    * **Feature:** Implemented temperature compensation logic for both pH and EC measurements.
    * **Feature:** Enhanced the `AdcManager` with "Smart ADC" logic, allowing probes to be put into a low-power dormant state by manipulating the ADC's single-shot and continuous modes.
    * **Unit Test:** Created and passed a comprehensive unit test for the `CalibrationManager`, validating its mathematical accuracy.
    * **Integration:** Updated `main.cpp` to create a full data pipeline: Raw ADC -> `FilterManager` -> `CalibrationManager` -> Final Compensated Value.
    * **Integration:** Enhanced the `telemetryTask` to display the output of every stage of the data pipeline, providing a powerful tool for live integration testing.
* **v2.3.0 (2025-07-27):**
    * **Status:** Complete.
    * **Milestone:** Successfully implemented and integrated the `FilterManager` cabinet.
    * **Feature:** Created the core `PI_Filter` class, encapsulating the Median and Stateful PI filtering logic as per the user manual and architectural blueprint.
    * **Feature:** Created the `FilterManager` cabinet to manage the two-stage (HF/LF) filtering pipeline.
    * **Fix:** Debugged and corrected a subtle logic flaw in the `PI_Filter`'s median calculation, enabling the unit test to pass and validating the filter's core functionality.
    * **Integration:** Integrated the `FilterManager` into `main.cpp`, routing raw ADC data through the new filtering pipeline and updating the `telemetryTask` to report the clean, filtered voltage.
* **v2.2.0 (2025-07-26):**
    * **Status:** Complete.
    * **Milestone:** Successfully integrated the `PowerMonitor` cabinet and its `INA219_Driver` HAL into the main application.
    * **Feature:** Created a new `i2cTask` to periodically call `powerMonitor.update()`, ensuring the battery gauge is always current.
    * **Feature:** Created a new `sensorTask` to perform periodic raw data acquisition from the ADCs.
    * **Feature:** Implemented a `telemetryTask` to gather and report processed data from all managers to the serial monitor, confirming end-to-end data flow.
    * **Enhancement:** Integrated the live State of Charge (SOC) from the `PowerMonitor` into the `DisplayManager`'s boot screen for immediate visual feedback.
* **v2.1.28 (2025-07-26):**
    * **Status:** Complete.
    * **Fix:** Resolved the final, persistent `SdManager` initialization failure by implementing the **explicit bus arbitration** strategy discovered from the legacy project. All SPI device managers now manually de-select other slaves before initiating a transaction, ensuring a stable, conflict-free shared bus.
    * **Milestone:** All foundational hardware (ADCs, SD Card) are now initializing and functioning correctly. The project is unblocked and ready for application-level development.
* **v2.1.27 (2025-07-26):**
    * **Status:** Complete.
    * **Fix:** Implemented a robust, mutex-protected, shared SPI bus architecture, mirroring the successful pattern from the legacy project.
* **v2.1.26 (2025-07-25):**
    * **Status:** Complete.
    * **Fix:** Successfully implemented the **"Intentional Bus Priming"** strategy discovered from the legacy project. A "dummy read" is now performed on an ADC immediately after the `AdcManager` is initialized. This stabilizes the SPI bus and allows the `SdManager` to initialize successfully.
    * **Milestone:** The primary goal of this session is now complete. The firmware correctly initializes all foundational hardware components, respects the "SPI Bus Precedence Rule", and successfully reads data from both ADCs and the SD Card.
* **v2.1.25 (2025-07-25):**
    * **Status:** Complete.
    * **Fix:** Corrected the SPI clock speed for the SD card to a safer 4 MHz, which improved compatibility and was a necessary step for successful initialization.
* **v2.1.23 (2025-07-25):**
    * **Status:** Complete.
    * **Fix:** Resolved both the `SdManager` initialization failure and the zero-value ADC readings.
        1.  Added SPI transaction control (`beginTransaction`/`endTransaction`) to the `AdcManager` to fix the `0.00 mV` readings, as required by the custom driver.
        2.  Performed a "dummy read" after initializing the `AdcManager` to properly prime the SPI bus, which allowed the `SdManager` to initialize successfully.
    * **Milestone:** The firmware now correctly initializes all foundational hardware components, respects the "SPI Bus Precedence Rule", and successfully reads data from the ADCs. The primary goal of this session is now complete.
* **v2.1.22 (2025-07-25):**
    * **Status:** Complete.
    * **Fix:** Resolved a critical `Guru Meditation Error` (LoadProhibited) that was causing a fatal crash on boot. The issue was a null pointer dereference in the `AdcManager` caused by an incorrect initialization order. The fix involved changing the `ADS1118` objects to pointers and creating them dynamically after the SPI bus was initialized.
* **v2.1.21 (2025-07-25):**
    * **Status:** Complete.
    * **Milestone:** All unit tests for foundational cabinets (`FaultHandler`, `ConfigManager`, `DisplayManager`, `AdcManager`, `SdManager`) are now passing. The persistent, hardware-level `[upload] Error 2` has been resolved by reinstalling the USB-to-serial drivers, unblocking all future development.
* **v2.1.20 (2025-07-25):**
    * **Status:** Complete.
    * **Fix:** Resolved the persistent `[upload] Error 2` by performing a clean reinstallation of the USB-to-serial (CP210x) driver, which fixed a driver conflict that made the COM port unavailable during the upload process.
* **v2.1.19 (2025-07-25):**
    * **Status:** Failed
    * **Issue:** An attempt to bypass the test framework and perform a direct firmware upload also failed. The error `Could not open COMx, the port doesn't exist` confirmed the issue is with the uploader's access to the port, not the test environment.
* **v2.1.18 (2025-07-25):**
    * **Status:** In Progress
    * **Task:** Created and integrated a new `AdcManager` cabinet to manage the ADS1118 ADCs. Updated `main.cpp` to perform a hardware-level test to read and display ADC values, verifying sensor functionality before addressing the `SdManager` initialization issue.
* **v2.1.17 (2025-07-25):**
    * **Status:** In Progress.
    * **Task:** Initiating a new session to resolve the persistent `SdManager` initialization failure by correctly implementing the "SPI Bus Precedence Rule".
* **v2.1.16 (2025-07-25):**
    * **Status:** Failed.
    * **Issue:** The attempt to fix the `SdManager` initialization by priming the SPI bus was unsuccessful, indicating a deeper issue in the boot sequence or hardware interaction.
* **v2.1.15 (2025-07-25):**
    * **Status:** Failed.
    * **Issue:** The `SdManager` failed to initialize upon boot. The issue was incorrectly diagnosed as a hardware problem and then correctly identified as a software flaw.
* **v2.1.14 (2025-07-25):**
    * **Status:** Complete.
    * **Milestone:** All unit tests for the foundational cabinets (`FaultHandler`, `ConfigManager`, `DisplayManager`, `SdManager`) are passing. The hardware upload and communication issues have been resolved. The project is no longer roadblocked and has a stable development environment.
* **v2.1.13 (2025-07-25):**
    * **Status:** Complete.
    * **Fix:** Resolved persistent hardware-level `[upload] Error 2`.
* **v2.1.11 (2025-07-25):**
    * **Status:** In Progress.
    * **Fix:** Addressed the persistent `[upload] Error 2` by specifying a robust `upload_resetmethod` in `platformio.ini`. This should resolve the communication issue between the uploader and the ESP32 board.
* **v2.1.10 (2025-07-25):**
    * **Status:** In Progress.
    * **Fix:** Implemented a robust fix for the persistent library compilation errors.
        * Consolidated the `I_StorageProvider` interface into the `SdManager` library to resolve dependency issues.
        * Created a custom `SdFatConfig.h` file to configure the `SdFat` library for the ESP32 and resolve header definition conflicts.
* **v2.1.9 (2025-07-25):**
    * **Status:** In Progress.
    * **Fix:** Resolved critical build and upload errors.
        * Added `upload_port = AUTO` to `platformio.ini` to fix the upload port detection issue.
        * Refactored the `lib` directory to use the standard `lib/libname/src` structure, resolving the compilation error for the `SdManager`.
        * Updated `SdFat` library to version `2.3.0` as specified.
* **v2.1.8 (2025-07-25):**
    * **Status:** Complete.
    * **Task:** Implemented the foundational storage layer.
        * Created the `I_StorageProvider` interface to decouple cabinets from the storage medium.
        * Added the `SdFat` library for high-performance file I/O.
        * Created the `SdManager` cabinet as the concrete implementation for the SD card.
        * Created the initial unit test for the `SdManager`.
* **v2.1.7 (2025-07-25):**
    * **Status:** Complete.
    * **Milestone:** All foundational unit tests for `FaultHandler`, `ConfigManager`, and `DisplayManager` are now passing.
* **v2.0.0 (2025-07-22):**
    * Established the complete foundational software architecture.










## Current Task (What We Are Doing)

* **UI Development - Phase 2: Boot Selection Logic (BLOCKED)**. The project is currently blocked by a persistent compilation error related to header include paths for the `InputManager` library. Multiple attempts to resolve this via relative paths and `library.json` manifests have been unsuccessful. The immediate goal is to resolve this build system issue before proceeding.











## Roadmap (What Is to Come)

1.  **UI - Phase 2: Boot Selection Logic (CONTINUE)**: Once unblocked, continue the implementation of the dual-boot selection menu.
2.  **UI - Phase 3: pBios Boot UI**: Build the dedicated user interface for the pBios diagnostics and tuning environment.
3.  **UI - Phase 4: Normal Boot UI**: Build the main application's user interface, including the main menu, live measurement screens, and the full graphical calibration wizard.
4.  **Implement the `StatusIndicatorController`**: Implement the logic to drive the physical LEDs and on-screen status icons based on system state.
5.  **Develop the `ConnectivityManager` and API Layer**: Implement the Wi-Fi/BLE connection management and the remote control API.