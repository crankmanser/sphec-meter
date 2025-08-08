// File Path: /pBIOS_Architecture.md
// NEW FILE

# Architecture Blueprint: The pBIOS Tuning Workbench v3.1

## 1. Core Philosophy & Architecture

The pBIOS UI moves away from a simple, monolithic auto-tuner and evolves the feature into a comprehensive **"Tuning Workbench"**. This architecture is built on the core principles of **modularity, user control, and immediate feedback**. It is designed to empower both the novice user who needs a guided, effective solution and the expert user who requires granular control and deep diagnostics.

Architecturally, this is achieved by breaking the `GuidedTuningEngine` into a suite of discrete, callable functions (e.g., `characterizeSignal()`, `optimizeHfStage()`) that can be used individually by expert tools or sequentially by a guided wizard.

---

## 2. Practical Implementation & RTOS Architecture

To translate the ideal design into a stable and efficient implementation on the ESP32, we will adhere to three core rules that govern how we manage system resources and user interaction.

### 2.1. The Rule of Asymmetrical Memory Location (RAM vs. SD Card)

We will use a hybrid "hot/cold" storage system to drastically reduce the RAM footprint without compromising the user experience.

* **`working_params` (Hot Storage - RAM):** This will be the single, stateful `FilterManager` object (~4KB). It resides permanently in RAM to provide the instantaneous feedback required for live manual tuning.
* **`saved_params` (Cold Storage - SD Card):** This exists as a JSON file on the SD card and is **not** kept in RAM. It is loaded into a temporary, lightweight struct only when the "Restore Saved Tune" function is called, a process masked by a brief "Loading..." message.
* **`proposed_params` (Cold Storage - SD Card):** The `GuidedTuningEngine` writes its results to a temporary JSON file on the SD card. If the user accepts the proposal, this file is loaded and applied to the live `working_params`.

This approach reduces the persistent memory footprint of the entire three-tiered system from a prohibitive 12KB to a highly efficient **~4.1KB**.

### 2.2. The "What You See Is What You Get" Live Data Pipeline

The system will feel instantly responsive during manual tuning by using a highly optimized, dual-core data pipeline with minimal mutex lock times.

* **The Pipeline:** The `pBiosDataTask` (Core 0) runs continuously, processing raw ADC values through the `working_params` filter. The `pBiosUiTask` (Core 1) handles all user input and screen rendering.
* **Minimal Lock Time:** To prevent UI lag, the mutex protecting `working_params` is held for the shortest possible duration.
    * **UI Task (Write):** Locks, writes a single parameter change, and unlocks immediately.
    * **Data Task (Read):** Locks, makes a fast local copy of the parameters it needs for the current cycle, and unlocks immediately before starting any heavy computation.

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

* **Layout:** Standard block layout (Help on Top, Menu in Middle, Breadcrumbs/Status on Bottom).
* **Menu Items:** `Auto Tune`, `Manual Tune`, `Save Tune`, `Restore Tune`, `Exit`.

### 3.2. Auto Tune Sub-Menu

* **Layout:** Standard block layout.
* **Menu Items:** `Tuner Wizard`, `Signal Profile`, `HF Optimization`, `LF Optimization`, `Probe Correction`.

### 3.3. The Guided Auto Tuner Wizard

An interactive, step-by-step process. Live graphs are paused and replaced with progress bars and report cards.

* **Step 1: Signal Characterization:** Runs deep analysis. UI shows a "Report Card" with noise fingerprint.
* **Step 2: HF Optimization:** Runs iterative search. UI shows "Before vs. After" KPIs.
* **Step 3: Intermediate Re-Analysis:** Runs a second FFT on the virtually cleaned signal.
* **Step 4: LF Optimization:** Runs final iterative search. UI shows final "Before vs. After" KPIs.

### 3.4. Manual Tune Screen (`ParameterEditScreen`)

* **Layout:** This screen is an overlay. It displays live graphs on the top and bottom OLEDs (rendered by the `LiveFilterTuningScreen`) and its own parameter list in the middle.
* **Controls:** Encoder to scroll/edit, buttons for `Cancel`, `Set`, and `Edit/OK`.
* **Feature: "Compare Mode" (Optimized)**: When toggled on, the system performs a one-time, background simulation using the `saved_params` to generate a static "ghost" dataset for the graphs, ensuring minimal CPU load.