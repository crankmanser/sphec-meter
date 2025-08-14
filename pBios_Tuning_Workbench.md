# Architecture Blueprint: The pBIOS Tuning Workbench v3.1.3

## 1. Core Philosophy & Architecture

The pBIOS UI moves away from a simple, monolithic auto-tuner and evolves the feature into a comprehensive **"Tuning Workbench"**. This architecture is built on the core principles of **modularity, user control, and immediate feedback**. It is designed to empower both the novice user who needs a guided, effective solution and the expert user who requires granular control and deep diagnostics.

Architecturally, this is achieved by breaking the `GuidedTuningEngine` into a suite of discrete, callable functions (e.g., `characterizeSignal()`, `optimizeHfStage()`) that can be used individually by expert tools or sequentially by a guided wizard.

---

## 2. Practical Implementation: The "Statistical Snapshot" Algorithm

To translate the ideal design into a stable and efficient implementation on the ESP32, we will adhere to a core rule that governs how we manage system resources and user interaction: **The Rule of Safe, RAM-Based Analysis.**

### 2.1. The "Live Snapshot" Capture Method

We will use a **fast, safe, and statistically robust** capture method to acquire the signal data for tuning. This completely avoids the instability of disk-based operations and the memory risks of large RAM buffers.

* **Multi-Pass RAM Capture:** The `GuidedTuningEngine` performs three separate, short, high-speed captures of the live signal directly into the ESP32's fast RAM. Each capture is only a few kilobytes, making it extremely safe and immune to heap fragmentation or stack overflow issues.
* **Statistical Averaging:** These three captures are then averaged together point-by-point to create a single, high-confidence "mean" dataset. This process ensures the resulting "noise fingerprint" is not skewed by any random anomalies and is a highly reliable representation of the probe's real-world signal.

This approach is the definitive solution, providing the highest possible data quality for the tuning engine while respecting the memory and SPI bus limitations of the hardware.

### 2.2. The "What You See Is What You Get" Live Data Pipeline

The system will feel instantly responsive during manual tuning by using a highly optimized, dual-core data pipeline with minimal mutex lock times.

* **The Pipeline:** The `pBiosDataTask` (Core 0) runs continuously, processing raw ADC values through the live filter parameters. The `pBiosUiTask` (Core 1) handles all user input and screen rendering.
* **Minimal Lock Time:** To prevent UI lag, the mutex protecting the filter parameters is held for the shortest possible duration.
    * **UI Task (Write):** Locks, writes a single parameter change, and unlocks immediately.
    * **Data Task (Read):** Locks, makes a fast local copy of the parameters it needs for the current cycle, and unlocks immediately before starting any heavy computation. The `pBiosDataTask` also includes a consistent `22ms` delay in its processing loop to yield time to the RTOS scheduler, guaranteeing a smooth and responsive user interface free of freezes.

### 2.3. The Rule of Asymmetrical UI/UX Responsiveness

The system's responsiveness is tailored to the user's expectation for a given task.

* **Moments of Instantaneity (The "Hot" Path):** During `Manual Tune`, where the user expects 1:1 feedback, the UI is 100% live.
* **Moments of Deliberation (The "Cold" Path):** During complex, automated tasks like the `Auto Tune Wizard`, we "use time as a resource."
    * **Pause & Inform:** The live graphs are paused, and the UI is dedicated to a multi-stage **"smart" progress bar**.
    * **Dedicate Resources:** The `pBiosDataTask` can then safely dedicate all of Core 0's resources to the complex tuning algorithm without risk of watchdog timeouts.
    * **Maintain Control:** The `pBiosUiTask` remains "hot," allowing the user to **cancel** the long-running operation at any time.

---

## 3. UI/UX Workflow: Screens & Controls

The following section details the user-facing workflow, built upon the practical architecture defined above.

### 3.1. Main Tuning Screen (The Workbench Hub)

* **Layout:** A clean, data-rich layout. The top and bottom screens display live graphs and KPIs for the HF and LF filters. The middle screen contains the main navigation menu and the final, calibrated value (pH/EC).
* **Menu Items:** `Auto Tune`, `Manual Tune`, `Save Tune`, `Restore Tune`, `Exit`.

### 3.2. Auto Tune Sub-Menu

* **Layout:** Standard block layout.
* **Menu Items:** `Tuner Wizard`, `Signal Profile`, `HF Optimization`, `LF Optimization`, `Probe Correction`.

### 3.3. The Guided Auto Tuner Wizard

An interactive, step-by-step process. Live graphs are paused and replaced with progress bars and report cards.

* **Step 1: Signal Characterization:** Runs the multi-pass, RAM-based "Statistical Snapshot" capture. UI shows a "Report Card" with the final noise fingerprint.
* **Step 2: HF Optimization:** Runs iterative search. UI shows "Before vs. After" KPIs.
* **Step 3: LF Optimization:** Runs final iterative search on the same high-resolution data. UI shows final "Before vs. After" KPIs.

### 3.4. Manual Tune Screen (`ParameterEditScreen`)

* **Layout:** This screen is an overlay. It displays live graphs on the top and bottom OLEDs (rendered by the `LiveFilterTuningScreen`) and its own parameter list in the middle.
* **Controls:** Encoder to scroll/edit, buttons for `Cancel`, `Set`, and `Edit/OK`.
* **Feature: "Compare Mode" (Optimized)**: When toggled on, the system performs a one-time, background simulation using the `saved_params` to generate a static "ghost" dataset for the graphs, ensuring minimal CPU load.

### 3.5. Probe Analysis Screen (`ProbeProfilingScreen`)

* **Layout:** A data-rich, three-OLED "report card" that provides a comprehensive overview of a single probe's health.
* **Top OLED (Health KPIs):** Displays the most critical, direct indicators of probe health: `Zero-Point Drift (mV)` and `Live R_std (mV)`.
* **Middle OLED (Filter Load):** Displays the saved `settleThreshold` and `lockSmoothing` parameters for both HF and LF filters. This shows the "filter creep," an indirect measure of how hard the system is working to compensate for noise.
* **Bottom OLED (History):** Displays the `Last Calibrated Timestamp` and the `Calibration Quality Score (%)` to provide historical context for the probe's current state.