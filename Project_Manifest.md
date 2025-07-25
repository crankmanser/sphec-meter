# SpHEC Meter Project Manifest

This document tracks the development progress, current tasks, and future roadmap for the SpHEC Meter firmware.

## Changelog (What Was Done)

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

* **Verify `SdManager` Initialization.** With the boot sequence now correctly following the architectural constraints, we will build and upload the firmware to confirm that the `SdManager` initializes successfully.








## Roadmap (What Is to Come)

1.  Implement the `PowerMonitor` cabinet and its associated `i2cTask`.
2.  Implement the `FilterManagerHF` and `FilterManagerLF` cabinets.
3.  Develop the `adcTask` to read from the ADS1118 ADCs.
4.  Develop the `CalibrationManager` and the 3-point calibration UI wizard.
5.  Implement the `StatusIndicatorController`.
6.  Develop the `ConnectivityManager` and the API layer.
7.  Implement the full UI based on the `UIEngine` components.