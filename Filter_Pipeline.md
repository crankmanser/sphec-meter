# SpHEC Meter - Data Processing Pipeline v3.1.3
This document outlines the complete data processing pipeline for the SpHEC Meter, from raw sensor acquisition to the final, calibrated output. The pipeline is designed as a sequential, multi-stage process that ensures the highest possible signal quality and measurement accuracy.

## Stage 1: Hardware-Level Processing
This is the initial data acquisition stage, managed by the AdcManager.

**Raw Sensor Data:** The process begins with the raw analog voltage reading from the pH or EC probe. This signal is often noisy and contains high-frequency spikes and low-frequency drift.

**Priming Read:** The AdcManager performs two consecutive reads from the ADS1118 ADC, discarding the first result. This "priming read" is a critical hardware-level requirement to ensure the ADC's internal sampling capacitor is stabilized, providing a reliable second reading.

## Stage 2: pBIOS Filter Tuning (The "Tuning Workbench")
This is a one-time, offline process performed in the pBIOS environment to create an optimal set of filter parameters for a specific probe.

**"Statistical Snapshot" Capture:** The raw signal is captured by the `GuidedTuningEngine`. It performs a fast, safe, multi-pass capture directly into RAM, which is then averaged to create a high-confidence "noise fingerprint" of the signal. This method is immune to the memory and SPI bus limitations of the hardware.

**Holistic Optimization:** The engine uses this fingerprint to run a multi-stage, heuristic simulation to find the optimal parameters for both the HF and LF filter stages.

**User Fine-Tuning:** The user can accept the engine's proposal or use the manual tuning tools in the "Tuning Workbench" to perform final, fine-grained adjustments.

**Setpoint Storage:** The final, optimized PI Filter Parameters (setpoints) are saved permanently to a configuration file on the SD card.

## Stage 3: Live Filtering (Main Application)
This is the real-time filtering process that runs continuously during normal operation in the main boot environment.

**Two-Stage Filtering:** The raw, primed data from Stage 1 is processed by the `FilterManager`.

**HF Filter (Stage 1):** The signal first passes through the High-Frequency "Spike Scraper" filter, which uses the pBIOS-tuned setpoints to eliminate sharp, fast noise.

**LF Filter (Stage 2):** The now-cleaner signal passes through the Low-Frequency "Smoothing Squeegee" filter, which uses its own tuned setpoints to eliminate slow, long-term drift.

**Filtered Voltage Output:** The result of this stage is a clean, stable, and highly reliable voltage signal. This signal is the input for the final calibration stage.

## Stage 4: Calibration & Compensation (Main Application)
This is the final mathematical conversion stage.

**Calibration Model:** The stable voltage from Stage 3 is fed into the `CalibrationManager`. It applies the active quadratic calibration model (y = ax² + bx + c) to convert the voltage into a scientific measurement (pH or EC).

**Temperature Compensation:** The `CalibrationManager` then applies a final temperature compensation algorithm to the scientific value, using live data from the `TempManager`.

**Final Output:** The result is the final, accurate, and temperature-compensated measurement that is displayed to the user.