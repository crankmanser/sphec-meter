# SpHEC Meter Project Manifest

This document tracks the development progress, current tasks, and future roadmap for the SpHEC Meter firmware.

## Changelog (What Was Done)

* **v2.0.0 (2025-07-22):**
    * Established the complete foundational software architecture based on a "Cabinet" philosophy.
    * Defined the specifications for all major engines: Filter, Calibration, Power Monitor, Storage, and UI.
    * Outlined the RTOS task structure, inter-task communication patterns, and key hardware constraints.
    * Generated the initial project files (`platformio.ini`, configuration headers, `main.cpp` skeleton) and architectural documents.

## Current Task (What We Are Doing)

* Implement the foundational "Cabinet" managers and helper classes (e.g., `DisplayManager`, `SdManager`, `ConfigManager`, `FaultHandler`). This involves creating the class structures and basic initialization logic.

## Roadmap (What Is to Come)

1.  Implement the `PowerMonitor` cabinet and its associated `i2cTask` for reading the INA219.
2.  Implement the `FilterManagerHF` and `FilterManagerLF` cabinets.
3.  Develop the `adcTask` to read from the ADS1118 ADCs and feed the data through the filter pipeline.
4.  Develop the `CalibrationManager` and the 3-point calibration UI wizard.
5.  Implement the `StatusIndicatorController` and integrate its output with the UI and physical LEDs.
6.  Develop the `ConnectivityManager` and the API layer.
7.  Implement the full UI based on the `UIEngine` components.