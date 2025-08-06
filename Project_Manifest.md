// File Path: /src/Project_Manifest.md
// MODIFIED FILE

# SpHEC Meter Project Manifest

This document tracks the development progress, current tasks, and future roadmap for the SpHEC Meter firmware.

## Changelog (What Was Done)

* **v2.11.22 (2025-08-06):**
    * **Status:** In Progress - BLOCKED
    * **Milestone:** Attempted to implement the `GuidedTuningEngine`'s heuristic algorithm.
    * **Issue (Critical):** The implementation has led to persistent and critical stability issues, including heap corruption and stack overflow errors within the `pBiosDataTask`. Multiple attempts to refactor the algorithm (using copy-safe objects, single-pass heuristics) have failed to produce a stable result. The feature is currently non-functional and is blocking further development of the pBIOS.

* **v2.11.21 (2025-08-06):**
    * **Status:** Complete.
    * **Milestone:** Refactored the pBIOS menu structure and implemented several stable maintenance features.
    * **Feature:** Implemented a pBIOS-safe "Hardware Self-Test" to verify the core components.
    * **Feature:** Implemented a "Live ADC Voltmeter" for low-level hardware diagnostics.
    * **Feature:** Implemented the foundational "Probe Profiling" screen, which successfully displays live `R_std` and saved "Filter Creep" data, though the UI layout requires minor cosmetic adjustments.
    * **Design:** Finalized a comprehensive redesign of the pBIOS menu structure to improve usability and provide a logical framework for new diagnostic tools.

* **v2.11.20 (2025-08-06):**
    * **Status:** Complete.
    * **Milestone:** The pBIOS "Guided Tuning" feature is now feature-complete, stable, and validated. The "setpoint editions" stage is complete.
    * **Feature:** Implemented the `GuidedTuningEngine`, a data-driven heuristic algorithm that performs statistical and FFT analysis on the live signal to automatically propose a high-quality baseline set of parameters for the HF and LF filter stages.
    * **Feature:** Implemented a robust persistence layer. Tuned setpoints can now be permanently saved to a configuration file on the SD card via the `ConfigManager` and are automatically loaded on boot.
    * **Fix (Critical):** Resolved all outstanding stability issues, including Guru Meditation Errors (memory corruption, incorrect FFT initialization) and Task Watchdog crashes (by re-architecting the tuning algorithm to be RTOS-friendly and non-blocking).
    * **Design:** Finalized a comprehensive redesign of the pBIOS menu structure to improve usability and provide a logical framework for new diagnostic tools.
* **v2.11.19 (2025-08-05):**
    * **Status:** Planning Complete.
    * **Milestone:** Completed a comprehensive review and redesign of the pBIOS "Live Filter Tuning" workflow. All outstanding bugs and UI/UX issues have been analyzed, and a definitive "Reset and Rebuild" plan has been formulated to ensure a stable and correct implementation.
    * **Design (Filter Pipeline):** Solidified the two-stage filter design. The HF stage is now specialized as a "Spike Scraper," and the LF stage is specialized as a "Smoothing Squeegee," each with distinct parameters to target different noise domains.
    * **Design (UI Workflow):** Finalized the "Guided Tuning" workflow. The system will now automatically analyze the signal upon entering the tuning screen and propose a high-quality baseline set of parameters, which the user can then fine-tune.
    * **Design (UI Layout):** Finalized a new, clean, and data-rich layout for the Live Filter Tuning screen that solves all previous visual conflicts and improves usability.
    * **Design (Storage):** Formalized the dual-save strategy. Essential filter setpoints and calibration models will be saved to the ESP32's internal NVS for fast boot and robust "Limp Mode" operation, while detailed tuning logs will be saved to the SD card for the companion Android app.

* **v2.11.18 (2025-08-03):**
    * **Status:** Complete.
    * **Milestone:** All major pBIOS diagnostic features are now implemented, stable, and feature-complete. The firmware is ready to begin development on the main application's user interface.
    * **Fix (Critical):** Resolved all outstanding bugs in the "Live Filter Tuning" workflow.
        * The pBIOS boot sequence now correctly initializes all required managers (`SdManager`, `TempManager`) and loads calibration files, fixing the `pH: -nan` error.
        * The standard deviation calculation in the `PI_Filter` was replaced with a numerically stable two-pass algorithm, permanently fixing the missing "R" & "F" KPI labels.
        * The data pipeline in `pBiosDataTask` now continues to run during parameter editing, ensuring the graphs on OLEDs #1 and #3 remain live and do not freeze.
        * The `ParameterEditScreen` was re-architected to be a true "overlay," fixing all text-overlap issues on the top and bottom screens during editing.
    * **Feature:** Implemented the complete "NA Drift Trending" screen, including a multi-step UI for configuration and a results screen that displays the top 3 dominant low-frequency peaks identified by an FFT analysis.
    * **Feature:** Implemented the complete "Noise Analysis" screen, including a multi-step UI, a non-linear "smart" progress bar, and a redesigned, data-rich results screen.
    * **Fix (Bootloader):** Re-architected the boot sequence to use a robust, legacy-inspired method that is immune to input "bleed-through" and timing sensitivity issues.
* **v2.11.17 (2025-08-02):**
    * **Status:** Complete.
    * **Milestone:** The pBIOS "Live Filter Tuning" screen now correctly displays the live, calibrated pH value.
    * **Fix (Critical):** Resolved a bug causing a "-nan" value to be displayed for pH during live tuning. The pBIOS boot sequence was failing to initialize the `SdManager` and `TempManager`, and did not load the calibration file from the SD card. These have been added to the pBIOS startup sequence, making the full data pipeline operational.
* **v2.11.16 (2025-08-02):**
    * **Status:** Complete.
    * **Milestone:** The pBIOS UI now provides a seamless and responsive user experience during filter tuning.
    * **Fix (Critical):** Resolved a major bug where the live graphs would freeze during parameter editing. Re-architected the main UI loop to ensure the graph data is updated every frame, and the edit screen is correctly rendered as an overlay, keeping all visual elements live and responsive.
    * **UI/UX:** Redesigned the status display on the "Live Filter Tuning" screen to be a cleaner, single line of essential data, as per user feedback.
* **v2.11.15 (2025-08-02):**
    * **Status:** Complete.
    * **Milestone:** The "Live Filter Tuning" workflow is now fully functional and feature-complete.
    * **Fix (Critical):** Resolved a major UI logic bug that prevented filter parameter values from being edited. The logic to prepare the `ParameterEditScreen` was tied to the wrong button event in the main UI task. This has been corrected, and editing filter values now works as intended.
    * **Enhancement:** The `pBiosDataTask` now includes the full data processing pipeline (Filter->Calibrate->Compensate) during live tuning, providing the final, human-readable value on the screen as described in the user manual.
* **v2.11.14 (2025-08-02):**
    * **Status:** Complete.
    * **Milestone:** The pBIOS "Drift Trending" (FFT analysis) screen is now fully implemented and feature-complete. All major pBIOS diagnostic tools are now functional.
    * **Feature:** Implemented the full background processing logic for the `DriftTrendingScreen`, including long-duration sampling, Hamming windowing, and FFT computation.
    * **UI/UX:** The `DriftTrendingScreen` now displays the top 2 most dominant low-frequency peaks, providing actionable data for diagnosing slow signal drift.
    * **Fix (Critical):** Resolved a system freeze/crash in the "Noise Analysis" screen by replacing the complex "smart" progress bar logic with a simpler, more robust sampling loop that correctly yields to the RTOS scheduler. This prevents watchdog timeouts and ensures system stability.
* **v2.11.13 (2025-08-02):**
    * **Status:** In Progress.
    * **Milestone:** Began implementation of the pBIOS "NA Drift Trending" screen.
    * **Feature:** Created the header and C++ files for the `DriftTrendingScreen`, defining its multi-step state machine (source select, duration select, sampling, analyzing, results) and data structures for storing FFT results. The UI for the configuration steps is complete.
    * **Architecture:** Added a new `DRIFT_TRENDING` state to the `StateManager` to formally integrate the screen into the UI engine.
* **v2.11.12 (2025-08-02):**
    * **Status:** Complete.
    * **Milestone:** The pBIOS boot sequence is now fully stable and reliable, mirroring the robust logic from the legacy firmware.
    * **Fix (Critical):** Resolved a persistent "input bleed-through" bug by re-architecting the `setup()` sequence. The firmware now waits for the user to release the boot-combo button and explicitly clears the `InputManager`'s state before starting the main UI tasks.
    * **UI/UX:** Redesigned the "Noise Analysis" results screen to display a clear statistical summary instead of a raw data graph, improving usability.
* **v2.11.11 (2025-08-02):**
    * **Status:** In Progress.
    * **Fix:** Reverted the boot selection logic to a simple, reliable `digitalRead()` at power-on, resolving a critical bug that prevented pBIOS mode from being accessible. The `BootSelector` is now a simple animation runner again. *(Self-correction: This fix was insufficient and was superseded by v2.11.12)*.
* **v2.11.10 (2025-08-02):**
    * **Status:** Complete.
    * **Milestone:** The bootloader has been re-architected to be a fully isolated, self-contained UI, resolving all input "bleed-through" and timing sensitivity issues.
    * **Fix (Critical):** Implemented a blocking, pre-RTOS boot selection menu that polls for input directly, inspired by the robust legacy firmware. The main application and RTOS tasks are now only started *after* a boot mode has been explicitly selected.
    * **Fix (Compiler):** Corrected a file path issue that prevented the `boot_sequence` module from finding the `InputManager` header.
* **v2.11.9 (2025-08-02):**
    * **Status:** Complete.
    * **Milestone:** The pBIOS "Noise Analysis" screen is now functional, providing statistical analysis and a graphical view of the raw signal.
    * **Feature:** Implemented a progress bar for the "Noise Analysis" sampling phase, providing better user feedback during the ~1 second data acquisition period. Created a new reusable `ProgressBarBlock` for this purpose.
    * **Feature:** The "Noise Analysis" results screen now displays a graph of the captured raw signal, allowing for visual inspection of the noise.
    * **Fix:** Corrected an issue where holding a button during boot to select a mode would cause that button press to be immediately registered by the UI. The firmware now waits for the button to be released before starting the UI task.
    * **Fix:** Made the `GraphBlock` more robust; it can now correctly render a single data series without crashing.
* **v2.11.8 (2025-08-02):**
    * **Status:** Complete.
    * **Milestone:** The foundational back-end for the pBIOS "Noise Analysis" screen is now implemented and integrated.
    * **Fix (Linker Error):** Restored the `uiTask` function definition in `main.cpp`, which was accidentally deleted, resolving a critical linker error.
    * **Feature:** The `pBiosDataTask` can now perform high-speed data sampling and statistical calculations when triggered by the `NoiseAnalysisScreen`.
    * **Integration:** Fully integrated the `NoiseAnalysisScreen` into the pBIOS state machine.
* **v2.11.7 (2025-08-02):**
    * **Status:** Complete.
    * **Milestone:** The Live Filter Tuning screen is now fully operational and correctly displaying live data.
    * **Fix (Critical):** Resolved a major logic error where the `FilterSelectionScreen` was not setting the shared `pBiosContext`. This prevented any data from being processed or displayed on the tuning screen, resulting in a flat line graph and zeroed KPI values. The context is now correctly set, fully enabling the pBIOS data pipeline.
    * **Feature:** Implemented the display of real-time filter KPIs (`R_std`, `F_std`, `Stab %`) on the `LiveFilterTuningScreen`, providing essential quantitative feedback for data-driven tuning as specified in the user manual.
* **v2.11.6 (2025-08-01):**
    * **Status:** Complete.
    * **Milestone:** The foundational pBIOS UI is now feature-complete, stable, and visually polished. All critical bootloader and input system failures have been resolved.
    * **Feature:** Implemented a complete, generic "Live Filter Tuning" workflow, allowing the user to select any of the four sensor filters and tune its parameters in real-time.
    * **Feature:** Created a dedicated `ParameterEditScreen` for a focused, user-friendly editing experience, as per the refined UI design.
    * **UI Polish:** Refined the pBIOS UI layout by moving menus to create better visual separation, implementing inverse-color button prompts for clarity, and creating a dedicated status area on the tuning screen.
    * **Fix (Bootloader):** Re-architected the dual-boot system to use the stable, legacy method of checking for a button press on power-up. This completely resolves the hardware state conflicts and instability of the previous reboot-based approach.
    * **Fix (Encoder):** The encoder is now fully functional in all modes. The boot menu uses simple polling, and the main/pBIOS applications use a robust, globally initialized `InputManager` based on the proven legacy ISR code.

* **v2.11.5 (2025-07-31):**
    * **Status:** Complete.
    * **Milestone:** Successfully resolved persistent, critical bootloader and input system failures. The pBIOS mode is now fully accessible and the encoder is functional in all UI modes.
    * **Fix (Bootloader):** Re-architected the dual-boot system to use a full reboot between mode selections. The user's choice is now saved to `RTC_NOINIT_ATTR` memory, which correctly persists the value across software resets and guarantees a clean hardware state for each boot mode.
    * **Fix (Encoder):** Resolved the unresponsive encoder by replacing the faulty `InputManager` implementations with a direct port of the proven ISR (Interrupt Service Routine) and velocity engine from the legacy v8.2.9 codebase.
    * **Feature:** Implemented the foundational pBIOS UI structure, including a dual-core RTOS architecture (`pBiosUiTask` on Core 1, `pBiosDataTask` on Core 0) and a `FilterSelectionScreen` to make the tuning process generic.
* **v2.11.4 (2025-07-31):**
    * **Status:** Planning Complete.
    * **Milestone:** Completed a comprehensive design and planning session for the advanced pBIOS diagnostics UI.
    * **Design:** Finalized a detailed, multi-screen UI layout for the "Live Filter Tuning" (LFT) feature, which will provide real-time graphical feedback for both HF and LF filter stages simultaneously.
    * **Design:** Defined a complete set of Key Performance Indicators (KPIs) for both real-time tuning (`F_std`, `R_std`, `Stab %`) and historical probe health (`Sensor Drift`, `Zero-Point Drift`, `Settling Time`).
    * **Architecture:** Formalized a set of key architectural refinements to be implemented, including a unified `ConfigManager`, a thread-safe `GlobalDataModel`, and a `SystemStatus` manager for non-fatal error handling.
* **v2.11.3 (2025-07-31):**
    * **Status:** Complete.
    * **Milestone:** Successfully refactored the entire UI architecture into a stable, three-part system, resolving all outstanding bugs.
    * **Feature:** Implemented a completely self-contained "Boot UI" in the boot_sequence module. It handles its own input polling and display logic, making the boot selection process extremely robust.
    * **Feature:** Created two distinct UI engines that are launched based on the boot mode:
    A simple, blocking "pBIOS UI Engine" for diagnostics mode.
    The full, RTOS-driven "Main UI Engine" for normal operation.
    * **Fix:** Resolved all UI bugs, including screen flickering, incorrect menu scrolling, and missing button prompts. The UI is now stable, responsive, and ready for feature development.
    * **Enhancement:** Tuned the encoder's velocity engine to provide a more intuitive and less sensitive feel, as per user feedback.
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

* **Diagnose and Resolve Critical Memory Crash in `GuidedTuningEngine`.**
    * **Goal:** To create a 100% stable and memory-safe implementation of the `GuidedTuningEngine`.
    * **Problem:** The current implementation, which runs during the `AUTO_TUNING_ANALYSIS` state, causes a fatal crash (either heap corruption or stack overflow) in the `pBiosDataTask`.
    * **Next Step:** A full architectural review of the `GuidedTuningEngine`'s memory usage patterns is required. The algorithm must be redesigned to be compatible with the memory constraints and real-time requirements of the ESP32 FreeRTOS environment. All further pBIOS development is blocked until this stability issue is resolved.













## Roadmap (What Is to Come)

1.  **Complete pBIOS Feature Set:**
    * a. Implement the `Probe Analysis` screen to display live `R_std` and saved "Filter Creep" data.
    * b. Implement the `New Probe` utility to reset configurations.
    * c. Implement the `pBIOS Snapshot` function to save a diagnostic file.
    * d. Implement the `SD Card Formatter` utility.
    * e. Implement the `Shutdown` screen logic, including the "safe to power off" message.

2.  **UI - Phase 4: Begin Normal Boot UI:**
    * a. Create the `Live Measurement Screen`, which will be the primary user-facing display.
    * b. Begin implementation of the multi-step `Calibration Wizard Screen`, including the guided "Hardware Calibration" for setting the amplifier's 2500 mV zero-point.

3.  **Implement the `SystemStatus` Manager** for detecting and reporting non-fatal errors like "Probe Disconnected."

4.  **Implement the `ConnectivityManager`** for the Android Suite feedback loop.