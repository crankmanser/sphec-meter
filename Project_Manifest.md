# SpHEC Meter Project Manifest

This document tracks the development progress, current tasks, and future roadmap for the SpHEC Meter firmware.

## Changelog (What Was Done)

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
## Changelog (What Was Done)
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

* **Session Complete.** We have successfully integrated the `PowerMonitor` and established the core application tasks (`sensorTask`, `i2cTask`, `telemetryTask`). The system is stable and producing live data.









## Roadmap (What Is to Come)

1.  **Implement the Filter Engine**: Create a `FilterManager` cabinet to process raw ADC voltages into clean, stable signals using the two-stage filtering logic.
2.  **Implement the Calibration Engine**: Create a `CalibrationManager` to convert the filtered voltages into final, calibrated pH and EC values using the quadratic model.
3.  **Enhance `telemetryTask`**: Update the telemetry output to report the final, scientifically accurate pH and EC values.
4.  **Develop the `StatusIndicatorController`**: Implement the logic to drive the physical LEDs and on-screen status icons based on system state.
5.  **Develop the `ConnectivityManager` and API Layer**: Implement the Wi-Fi/BLE connection management and the remote control API.
6.  **Implement the Full UI**: Build out the user interface screens based on the `UIEngine` components.
