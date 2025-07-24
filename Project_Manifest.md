# SpHEC Meter Project Manifest

This document tracks the development progress, current tasks, and future roadmap for the SpHEC Meter firmware.

## Changelog (What Was Done)

* **v2.1.2 (2025-07-24):**
    * **Status:** ROADBLOCKED.
    * **Issue:** Persistent `InvalidProjectConfError` due to a malformed `platformio.ini` file. All attempts to fix the file via copy-pasting new configurations have failed. The core build environment is unable to read the project configuration.
* **v2.1.1 (2025-07-24):**
    * Initiated a new session to resolve critical build environment issues.
* **v2.1.0 (In Progress):**
    * **Task**: Implement the foundational `DisplayManager` cabinet.
        * Created `DisplayManager.h` and `DisplayManager.cpp` to manage the three OLED screens via the I2C multiplexer.
        * Established the unit testing framework for the cabinet.
    * **Task**: Integrate the `ConfigManager` into the main application.
        * Created a global `configManager` instance and initialized it in `setup()`.
    * **Task**: Implement the foundational `ConfigManager` cabinet.
        * Created `ConfigManager.h` and `ConfigManager.cpp` and passed its initial unit test.
    * **Task**: Integrate the `FaultHandler` into the main application.
        * Created a global `faultHandler` instance and initialized it in `setup()`.
    * **Task**: Implement the foundational `FaultHandler` cabinet.
        * Created `FaultHandler.h` and `FaultHandler.cpp` and passed its initial unit test.
* **v2.0.0 (2025-07-22):**
    * Established the complete foundational software architecture based on a "Cabinet" philosophy.
    * Defined the specifications for all major engines: Filter, Calibration, Power Monitor, Storage, and UI.
    * Outlined the RTOS task structure, inter-task communication patterns, and key hardware constraints.
    * Generated the initial project files (`platformio.ini`, configuration headers, `main.cpp` skeleton) and architectural documents.

## Current Task (What We Are Doing)

* **Resolve `InvalidProjectConfError`:** Before any further development can occur, we must fix the root cause of the configuration file error. All other tasks are on hold until `pio project config` can be run successfully.



## Roadmap (What Is to Come)

1.  Implement the remaining foundational "Cabinet" managers (e.g., `DisplayManager`, `SdManager`, `ConfigManager`).
2.  Implement the `PowerMonitor` cabinet and its associated `i2cTask`.
3.  Implement the `FilterManagerHF` and `FilterManagerLF` cabinets.
4.  Develop the `adcTask` to read from the ADS1118 ADCs.
5.  Develop the `CalibrationManager` and the 3-point calibration UI wizard.
6.  Implement the `StatusIndicatorController`.
7.  Develop the `ConnectivityManager` and the API layer.
8.  Implement the full UI based on the `UIEngine` components.