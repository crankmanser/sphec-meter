This document provides a deep dive into the concepts, logic, and ideologies of the legacy code files for the **Smart-EC-pH-Meter-v8.2.9-legacy**.

***

### Core Architecture & Entry Point

#### **Legacy Component:** `src/main.cpp`
* **Core Responsibility**: As the main entry point, this file orchestrates the entire system startup. Its duties include initializing hardware peripherals (I2C, SPI), creating all global "Cabinet" managers, setting up shared data structures with FreeRTOS mutexes and queues, checking for special boot modes (like pBIOS), performing critical recovery operations, and launching all the RTOS tasks.
* **Key Logic & Ideology**:
    * **Intentional Bus Priming**: The most critical ideology here is the "SPI Bus Precedence Rule". The `ADS_Manager` (for ADCs) is deliberately initialized *before* the `StorageEngine` (for the SD card) to electrically stabilize the shared SPI bus, which is essential for the SD card's sensitive initialization handshake to succeed.
    * **Atomic Write Recovery**: On boot, the system performs a crucial safety check. It looks for temporary configuration files (`*.tmp`) that might have been left over from a crash during a previous write operation. If found, it renames them to their correct filenames, ensuring data integrity and preventing boot failures from corrupted configs.
    * **Boot Mode Detection**: The system can start in either `BOOT_NORMAL` or `BOOT_PBIOS` mode. This is determined by checking if a specific combination of buttons is held down during startup, providing a gateway to a low-level diagnostic and configuration environment.
    * **Centralized Globals**: All major system objects (managers, task handles, RTOS primitives) are declared globally in this file. This acts as a single source of truth for the system's components, preventing linker errors and making dependencies explicit.
* **Architectural Flaws**:
    * **Global Namespace Pollution**: While centralizing globals prevents linker errors, it heavily pollutes the global namespace. This makes dependency tracking between modules difficult and less explicit than a system using dependency injection.
    * **Monolithic `setup()` Function**: The `setup()` function is overly large and has too many responsibilities. It initializes hardware, managers, performs the POST (Power-On Self-Test) sequence, and launches tasks. This logic could be broken down into more modular, focused functions (e.g., `init_hardware()`, `init_managers()`, `run_post()`, `create_tasks()`).
    * **Hardcoded POST Sequence**: The POST sequence, which tests LEDs and systems, is hardcoded directly within `setup()`. This display and logic code is tightly coupled with `main.cpp` and would be better encapsulated in a dedicated `POSTManager` cabinet.

---

### RTOS Tasks & Concurrency

The system's functionality is divided across several FreeRTOS tasks, adhering to a dual-core architecture that separates concerns for stability and responsiveness.

#### **Legacy Component:** `src/tasks/uiTask.cpp` & `uiTask.h`
* **Core Responsibility**: This is the master task for the user interface, running on **Core 1** with a high priority. It manages the main application loop, which involves updating the button and notification managers, polling the current screen for state changes, handling user input, and orchestrating the rendering of all three OLED displays.
* **Key Logic & Ideology**:
    * **The "Switchyard"**: This task contains the critical rendering logic that differentiates between screen types. It checks if the current screen is a modern "Declarative" screen, an older "Profile-based" screen, or a legacy screen, and calls the appropriate rendering engine (`UIManager` or `UIEngine`) or the screen's own `draw()` method. This allows for a gradual refactoring of the UI without breaking older screens.
    * **Ownership of I2C Bus**: It takes and releases the `g_i2c_bus_mutex` during the rendering phase, asserting its ownership over the I2C bus to prevent conflicts with other tasks while communicating with the OLEDs and RTC.
    * **State-Driven Updates**: The task doesn't draw on every loop. It only triggers a redraw if the current screen's "dirty" flag is set, which is an efficient way to manage display updates. The flag is set by user input, screen state changes, or a once-per-minute timer.
* **Architectural Flaws**:
    * **Mixed Rendering Logic**: The "switchyard" logic, while necessary for the refactor, mixes concerns. The `uiTask` has to know about different types of screens and rendering engines. In a fully declarative model, it would only ever interact with one engine.
    * **Direct Screen Interaction**: The task directly calls methods on the `Screen` object (`update`, `handleInput`, `isDirty`). A more decoupled approach might use an event-based system where the `uiTask` dispatches events and the active screen subscribes to them.

#### **Legacy Component:** `src/tasks/sdTask.cpp` & `sdTask.h`
* **Core Responsibility**: This task is the sole gatekeeper for all SD card file operations. It runs on **Core 1** and spends most of its time blocked, waiting for `FileOperationRequest` messages to arrive on one of two queues (`g_sd_high_prio_queue` or `g_sd_normal_prio_queue`).
* **Key Logic & Ideology**:
    * **Priority Elevation for Stability**: This is the most critical concept. To prevent a "priority inversion deadlock" with the high-priority `uiTask` on the same core, the `sdTask` temporarily elevates its own priority to `SD_TASK_PRIORITY_HIGH` before any slow, blocking operation (like `WRITE_FILE` or `DELETE_FILE`). This ensures the file operation is uninterruptible and prevents system freezes, a crucial lesson documented in the project's hardware constraints.
    * **Asynchronous I/O**: By using a queue, other tasks can "fire-and-forget" file requests without blocking their own execution, which is essential for maintaining UI responsiveness. The requesting task can optionally provide a semaphore to be signaled upon completion for synchronous-style operations.
* **Architectural Flaws**:
    * **Dual Queues Underutilized**: While the architecture supports high and normal priority queues, the high-priority queue is not currently used in the provided code, representing an incompletely realized feature.

#### **Legacy Component:** `src/tasks/sensorTask.cpp` & `sensorTask.h`
* **Core Responsibility**: Running on **Core 0**, this task acts as the central data aggregator. It reads raw values from the ADCs (via `ADS_Manager`), gathers temperature data from the shared `g_one_wire_data` struct, and then delegates all processing and calibration to the `SensorProcessor` cabinet. Finally, it publishes the final, processed data to the `g_processed_data` struct for the UI to consume.
* **Key Logic & Ideology**:
    * **Delegation over Calculation**: The `sensorTask`'s primary ideology is to *delegate*, not to calculate. It is purely a data orchestrator. All complex logic (voltage conversion, filtering, calibration) is owned by the `SensorProcessor` and `CalibrationManager` cabinets. This is a clean separation of concerns.
    * **On-Demand Measurement Control**: The task's operation is governed by the global `g_is_pbios_active` and `g_probe_measurement_active` flags. This allows the system to turn off power to the probes when not in a measurement screen, significantly increasing their lifespan.
* **Architectural Flaws**:
    * **Slightly Misnamed**: While named `sensorTask`, its core responsibility has evolved into being more of a "processing and publishing" task. The actual sensor reading is now done by the `i2cTask`, `oneWireTask`, and directly within the `sensorTask` for the ADCs.

#### **Legacy Component:** `src/tasks/i2cTask.cpp` & `i2cTask.h`
* **Core Responsibility**: This task runs periodically on **Core 0** and is responsible for reading all sensors connected to the I2C bus. This includes the RTC (Real-Time Clock) and the INA219 power monitor IC. It then publishes this raw data to the `g_i2c_data` shared struct.
* **Key Logic & Ideology**:
    * **Multiplexer Management**: The task correctly handles the TCA9548A I2C multiplexer by calling `selectTCAChannel()` before communicating with the RTC. It also correctly understands that the INA219 is *not* on the multiplexer and reads it directly.
    * **Decoupled Power Monitoring**: After gathering I2C data, it calls `powerMonitor.update()`, feeding the power monitor the voltage and current data it needs to perform its own complex calculations (like coulomb counting). This is a great example of the Cabinet philosophy in action.
* **Architectural Flaws**:
    * **Hardcoded Delays**: The task uses a fixed `vTaskDelay` of 1 second. A more robust implementation might use an event-based trigger or a configurable delay.

#### **Legacy Component:** `src/tasks/oneWireTask.cpp` & `oneWireTask.h`
* **Core Responsibility**: This task runs on **Core 0** and handles all communication on the 1-Wire bus, periodically reading the DS18B20 liquid temperature sensor and the DHT11 ambient temperature/humidity sensor.
* **Key Logic & Ideology**:
    * **Applying Calibration Offsets**: This task is responsible for applying the user-configured temperature offsets. It retrieves the offsets from the `pBiosConfig` manager and applies them to the raw readings before publishing the final, corrected values to the `g_one_wire_data` struct.
    * **Graceful Sensor Failure**: The task includes checks to see if sensor readings are valid (e.g., not `NAN` or `DEVICE_DISCONNECTED_C`). If a sensor fails, it publishes `NAN` to the global data structure, allowing the UI to display "--" instead of crashing or showing garbage data.
* **Architectural Flaws**:
    * **Blocking Calls**: The `ds18b20.requestTemperatures()` call can be blocking. While the overall task delay is long (2 seconds), in a more time-critical system, a non-blocking approach might be preferred.

#### **Legacy Component:** `src/tasks/healthTask.cpp` & `healthTask.h`
* **Core Responsibility**: A simple but crucial diagnostic task that runs at a low priority. Its only job is to periodically check the "stack high-water mark" for every other task in the system and record it in the `g_system_health_data` struct.
* **Key Logic & Ideology**:
    * **Non-Intrusive Monitoring**: This task provides a way to monitor the memory usage of all other tasks without interfering with their operation. The "high-water mark" is a key RTOS metric that shows the minimum amount of free stack space a task has ever had, which is invaluable for detecting potential stack overflows.
* **Architectural Flaws**:
    * **No Action on Data**: The task only records the data. A more advanced implementation might trigger a warning or a safe-shutdown if a task's high-water mark drops below a critical threshold.

---

### "Cabinet" Modules

The project is built on a "Cabinet" philosophy, where each module is a self-contained unit responsible for a specific domain.

#### **Legacy Component:** `src/modules/storage/StorageEngine.cpp` & `StorageEngine.h`
* **Core Responsibility**: This is the master cabinet for all persistent data operations. It provides the **one and only** public interface for saving and loading configurations. It completely abstracts away file paths, JSON serialization, and RTOS-safe file access.
* **Key Logic & Ideology**:
    * **`ConfigType` Enum**: The core ideology is to replace scattered filename strings with a strong `ConfigType` enum. This makes all save/load operations type-safe and self-documenting. To save the pH calibration, one calls `storageEngine.save(ConfigType::PH_CALIBRATION)`.
    * **Scheduler-Awareness**: The `save()` method is aware of the RTOS scheduler's state. If called *before* the tasks have started (i.e., during `setup()`), it performs a direct, synchronous write. If called *after* tasks are running, it correctly uses the asynchronous `sdTask` queue to prevent blocking.
    * **Centralized Backup/Restore**: It contains the logic to back up all known configurations into a single, timestamped JSON archive and to perform a full factory reset by deleting all configuration files in a blocking, synchronous manner.
* **Architectural Flaws**:
    * **Potential for Large JSON Documents**: The `backupAllConfigs` method creates a single, large JSON document. For a system with many more configurations, this could become memory-intensive. A streaming or multi-file approach might be more scalable.
    * **Error in `save()` Implementation**: There was a subtle bug where the root `JsonObject` was not being correctly extracted from the `JsonDocument` before being passed to the serialization methods, which was fixed in the provided version.

#### **Legacy Component:** `src/modules/storage/SdManager.cpp` & `SdManager.h`
* **Core Responsibility**: A low-level cabinet that directly wraps the `SdFat` library. Its responsibility is to execute the basic file operations (read, write, delete, rename, mkdir) requested by the `sdTask`. **It is not intended to be called directly by any other module.**
* **Key Logic & Ideology**:
    * **Atomic Writes**: The `writeFile` method embodies the "write-to-temp, then-rename" pattern. It always writes content to a temporary file (e.g., `config.json.tmp`), and only upon successful completion does it delete the original and rename the temporary file. This guarantees that even if the device crashes mid-write, the original configuration file remains intact.
    * **Mutex Protection**: Every public method in this manager correctly takes and gives the `g_spi_bus_mutex`, ensuring that all its operations on the shared SPI bus are thread-safe.
* **Architectural Flaws**:
    * **Public Interface**: While intended for internal use by `sdTask`, its methods are public. In a stricter architecture, these might be private, with `sdTask` being a `friend` class, or the manager could be entirely encapsulated within the `sdTask.cpp` file.

#### **Legacy Component:** `src/modules/ui/UIEngine.cpp` & `UIEngine.h`
* **Core Responsibility**: The "Atelier." This cabinet is a centralized library of high-level, reusable UI components and the new declarative rendering engine.
* **Key Logic & Ideology**:
    * **Component-Based Drawing**: It provides standardized functions like `drawThreeLineMenu`, `drawFullscreenMessage`, and `drawLiveGraph`, which enforces a consistent visual theme and abstracts away low-level `Adafruit_GFX` drawing commands from the individual screen classes.
    * **Declarative Rendering**: Its main `render(const ScreenProfile& profile)` method is the heart of the new UI architecture. It takes a data structure describing what the screen should look like and handles the entire drawing process for all three OLEDs.
    * **Thread-Safe Notification System**: It contains a `_notification_pool` (a list of `Notification` objects) protected by a mutex. Any task in the system can safely call `uiEngine.pushNotification()` to display a message to the user. The engine itself handles queueing, checking for expired notifications, and rendering them according to priority.
* **Architectural Flaws**:
    * **Dual-Model Support**: Like the `uiTask`, it supports both the old (`drawThreeLineMenu`) and new (`render`) models, which adds complexity during the transition phase. The final goal would be to eliminate the older, direct-drawing methods.

#### **Legacy Component:** `src/modules/display/DisplayManager.cpp` & `DisplayManager.h`
* **Core Responsibility**: This is a Hardware Abstraction Layer (HAL) for the displays. Its job is to manage the three physical OLED screens connected via the TCA9548A I2C multiplexer. It provides basic functions like `selectOLED`, `clearAllDisplays`, and `displayAll`.
* **Key Logic & Ideology**:
    * **Multiplexer Abstraction**: The core concept is abstracting away the I2C multiplexer. Other modules don't need to know about the TCA9548A's address; they just call `displayManager.selectOLED(OLED1_TCA_CHANNEL)` to route I2C commands to the correct screen.
    * **Complex Composite Components**: It contains the logic for drawing complex, data-rich components like the `drawStatusArea`, which combines icons, text, and data from multiple sources into the consistent status bar seen on OLED #3.
    * **Flashing Icons for Urgency**: The `drawBatteryIcon` logic includes a stateful check using `millis()` to make the icon flash during a low-battery state, a simple but effective way to create a visual warning.
* **Architectural Flaws**:
    * **Mixing HAL and UI Logic**: The manager's responsibility blurs. While it should be a pure HAL, it contains significant UI drawing logic (e.g., `drawStatusArea`, `drawButtonPrompt`, `drawWifiIcon`). This high-level drawing logic belongs in the `UIEngine` or a similar UI-focused cabinet.

#### **Legacy Component:** `src/modules/processing/SensorProcessor.cpp` & `SensorProcessor.h`
* **Core Responsibility**: This cabinet is the brain of the sensor data pipeline. It is responsible for converting raw integer ADC values into meaningful floating-point voltages, applying advanced signal filtering, and then passing the clean voltages to the `CalibrationManager` to get final, calibrated pH and EC values.
* **Key Logic & Ideology**:
    * **Two-Stage Filtering**: It uses a sophisticated `PI_Filter` which is a Proportional-Integral filter combined with a median pre-filter. This provides excellent spike rejection and fast settling time, a key concept for stable sensor readings.
    * **Decoupling Raw Conversion and Calibration**: It strictly separates the task of converting raw ADC values to voltage from the task of converting voltage to a calibrated reading (pH, EC). This is a strong architectural choice, as it allows the calibration models to be updated without affecting the fundamental voltage conversion logic.
    * **State Persistence via Engine**: It correctly implements the `serializeFilterParams` and `deserializeFilterParams` methods, allowing the `StorageEngine` to manage the saving and loading of its tuning parameters without the `SensorProcessor` needing any knowledge of file I/O.
* **Architectural Flaws**:
    * **Deprecated Methods**: It still contains the old `loadFilterParams` and `saveFilterParams_blocking` methods. While kept for compilation during the refactor, these represent a "sin of the past" and should be removed, as they perform direct SD card access, violating the `StorageEngine`'s authority.

#### **Legacy Component:** `src/modules/calibration/CalibrationManager.cpp` & `CalibrationManager.h`
* **Core Responsibility**: This cabinet manages the entire lifecycle of a sensor's calibration. It stores calibration points, calculates a mathematical calibration model (y = ax² + bx + c), applies this model to convert clean voltages into scientific units, and assesses the health and drift of the probe over time.
* **Key Logic & Ideology**:
    * **Quadratic Regression Model**: It uses a quadratic equation (`y = ax² + bx + c`) to model the sensor's response curve, which is more accurate for many electrochemical sensors than a simple linear model.
    * **Scientific Temperature Compensation**: The `getCalibratedValue` method implements true scientific temperature compensation. For pH, it uses the Nernst equation slope adjustment. For EC, it uses a standard linear temperature coefficient. This is a core feature for an accurate instrument.
    * **Probe Health and Drift KPIs**: The manager calculates two vital Key Performance Indicators (KPIs):
        1.  **Quality Score**: An adaptive score based on the goodness-of-fit (R-squared) and slope sanity of a new 3-point calibration.
        2.  **Sensor Drift**: A "2D Integrated Deviation Analysis" that compares a new calibration curve against the previous one to quantify the probe's aging over time.
* **Architectural Flaws**:
    * **Coupled to Storage Path**: The `getCalibratedValue` method identifies whether it's dealing with pH or EC by checking its own `m_storage_path` string. This is a form of tight coupling; it would be cleaner if the manager was explicitly configured as a "pH" or "EC" type upon creation.

#### **Legacy Component:** `src/modules/power/PowerMonitor.cpp` & `PowerMonitor.h`
* **Core Responsibility**: This is an intelligent cabinet responsible for everything related to battery management. It tracks State of Charge (SOC), State of Health (SOH), charge cycles, and provides advanced diagnostics like DC Internal Resistance (DCIR).
* **Key Logic & Ideology**:
    * **Hybrid State Model**: It uses a sophisticated hybrid model. **Coulomb counting** is used for short-term accuracy, while **Open-Circuit Voltage (OCV) measurements** (using a lookup table) are used to correct for drift after long periods of rest. This combines the best of both methods.
    * **Stateful Learning Cycle (SOH Recalibration)**: This is a professional-grade feature. The monitor can detect a full charge/discharge cycle and measure the *true* delivered energy. It uses this data to learn the battery's actual health (SOH) and make the gauge more accurate over the battery's lifespan. This learning state is persistent and survives reboots.
    * **pBIOS Delta Reconciliation**: It accounts for energy consumed while the main application is not running (i.e., during a pBIOS session). It does this by reading a timestamped log file on the next boot and calculating the estimated power drain, ensuring the SOC remains accurate.
* **Architectural Flaws**:
    * **Direct File Access**: The `reconcileStateOnBoot` method performs direct SD card checks (`sd.exists`, `sd.rename`). This violates the `StorageEngine`'s authority and was a temporary measure during development. The final version should delegate all file I/O to the `StorageEngine`.

#### **Legacy Component:** `src/modules/io/ButtonManager.cpp` & `ButtonManager.h`
* **Core Responsibility**: This module handles all aspects of the physical buttons, including debouncing, detecting single presses, and detecting timed holds.
* **Key Logic & Ideology**:
    * **Debouncing**: It uses a standard `millis()`-based debounce algorithm to prevent a single physical press from registering as multiple presses.
    * **Stateful Hold Detection**: The `ButtonState` struct tracks not just the pressed state, but also `isHeld` and `holdStartTime`. This allows other parts of the system to easily query not just *if* a button is held, but for *how long*. This is crucial for features like the safe shutdown combo.
* **Architectural Flaws**:
    * **No Obvious Flaws**: This is a well-defined, focused, and robust hardware abstraction layer. It does its one job very well.

#### **Legacy Component:** `src/modules/io/EncoderManager.cpp` & `EncoderManager.h`
* **Core Responsibility**: This module manages the rotary encoder, using hardware interrupts for maximum accuracy. It translates the raw electrical pulses from the encoder into logical "UI steps" for menu navigation.
* **Key Logic & Ideology**:
    * **Interrupt-Driven**: It uses an Interrupt Service Routine (`isr`) to capture every single pulse from the encoder, ensuring no rotation is ever missed, regardless of what the main processor is doing.
    * **Speed-Sensitive Acceleration ("Speed Engine")**: This is its "secret sauce." The `getChange()` method doesn't just count pulses; it analyzes the *rate* of pulses. If the user turns the knob slowly, it requires more pulses per UI step (for fine control). If the user turns it quickly, it requires fewer pulses per UI step (for fast scrolling). This provides a very intuitive and responsive user experience.
* **Architectural Flaws**:
    * **No Obvious Flaws**: Like the `ButtonManager`, this is a very well-designed and focused hardware cabinet.

#### **Legacy Component:** `src/modules/io/LEDManager.cpp` & `LEDManager.h`
* **Core Responsibility**: This cabinet provides a clean interface for controlling the three status LEDs (Top Red, Top Green, Bottom Green).
* **Key Logic & Ideology**:
    * **Abstraction of Inverted Logic**: The hardware uses inverted logic (LOW = ON, HIGH = OFF). The `LEDManager` completely abstracts this away. The caller simply requests a logical state (`LedState::ON`, `LedState::OFF`, `LedState::PWM`), and the manager handles the underlying `ledcWrite` calls with the correct inverted values.
    * **PWM Control**: It correctly utilizes the ESP32's `ledc` (LED Control) peripheral to allow for Pulse-Width Modulation, enabling effects like "breathing" or dimming, not just simple on/off states.
* **Architectural Flaws**:
    * **No Obvious Flaws**: This is another example of a solid, well-defined hardware abstraction layer.

#### **Legacy Component:** `src/modules/adc/ADS_Manager.cpp` & `ADS_Manager.h` & `ADS1118.cpp`/`.h`
* **Core Responsibility**: The `ADS_Manager` is a custom, hardened cabinet that wraps the third-party `ADS1118` library. Its job is to manage communication with the two ADS1118 Analog-to-Digital Converters on the shared SPI bus.
* **Key Logic & Ideology**:
    * **Bus Arbitration**: The `getReading` method includes a call to `deselectOtherSlaves()`. This is a critical piece of logic that ensures only one device on the shared SPI bus is active at a time, preventing communication errors.
    * **Configuration Abstraction**: It abstracts away the complex configuration register of the ADS1118. The caller simply requests a reading (e.g., `readRaw_pH()`), and the manager handles setting the correct multiplexer input, gain, data rate, and operating mode for that specific measurement.
* **Architectural Flaws**:
    * **Third-Party Library (`ADS1118.cpp`)**: The underlying Denkitronik `ADS1118` library was found to have stability issues in a multi-tasking environment, which was the primary motivation for creating the `ADS_Manager` wrapper. The library's use of `SPI.transfer` without proper transaction management or mutex protection was a major source of bugs in earlier project versions.
    * **Blocking Delays**: The `getReading` method contains a `delay(15)` call. In a high-performance system, this blocking delay could be problematic. A non-blocking, asynchronous approach would be more robust.

---

### Documentation & Configuration

#### **Legacy Component:** `ARCHITECTURE.md`
* **Core Responsibility**: To provide a high-level overview of the project's software architecture, including the "Cabinet" philosophy, the FreeRTOS task structure, and the primary inter-task communication patterns (shared data with mutexes and asynchronous request queues).
* **Key Logic & Ideology**: Its purpose is to onboard new developers quickly and to serve as a "constitution" for the project, ensuring that future development adheres to the established architectural patterns. The Mermaid diagram is a key part of this, providing a clear visual representation of the system's data flow.

#### **Legacy Component:** `HARDWARE_CONSTRAINTS.md`
* **Core Responsibility**: To formally document critical, non-obvious hardware and RTOS rules that were discovered through painful debugging.
* **Key Logic & Ideology**: This document is the project's "Book of Sins." It exists to prevent the re-introduction of "ghost" bugs. The two primary constraints it documents are the "SPI Bus Precedence Rule" and the "`sdTask` Priority Inversion Rule," which are fundamental to the system's stability.

#### **Legacy Component:** `changelog.md`
* **Core Responsibility**: To maintain a chronological record of all significant changes, bug fixes, and feature additions for each version of the firmware.
* **Key Logic & Ideology**: The changelog is written for both users and developers. It not only describes *what* changed but also *why*, often including details from the root cause analysis of major bugs. This provides invaluable context for understanding the evolution of the codebase and the rationale behind key architectural decisions.

#### **Legacy Component:** `platformio.ini`
* **Core Responsibility**: This is the project's master configuration file for the PlatformIO build system. It defines the board, platform, framework, monitor speed, build flags, and all library dependencies.
* **Key Logic & Ideology**: The key ideology demonstrated in this version is the move *away* from a global include path (`-I include`) to explicit includes in each file. This makes dependencies clearer and the project more transparent and portable. The list of `lib_deps` serves as a manifest of all third-party code the project relies on.

#### **Legacy Component:** `src/core_config.h` & `src/debug_config.h` & `src/tuning_config.h`
* **Core Responsibility**: These files centralize all compile-time configurations.
    * `core_config.h`: Defines RTOS task priorities and core assignments.
    * `debug_config.h`: Provides a set of master switches to enable or disable debug logging for each individual module.
    * `tuning_config.h`: Defines the "factory default" parameters for the signal processing filters.
* **Key Logic & Ideology**: The primary ideology is **configuration over hardcoding**. By centralizing these values, they can be easily changed and experimented with without having to hunt through the entire codebase. The `debug_config.h` file, in particular, is a powerful tool for isolating bugs by selectively enabling logs from specific parts of the system.
* **Architectural Flaws**:
    * **Compile-Time Configuration**: While useful, these are compile-time constants. A more flexible system might load some of these values from a configuration file on the SD card at runtime, allowing for easier tuning without recompiling the firmware.


---


***

### pBios (Pseudo Basic Input/Output System)

The pBios is a self-contained, low-level diagnostic and recovery environment, completely separate from the main application. It is entered by holding a specific button combination during boot.

#### **Legacy Component:** `src/pBios/pBios.cpp` & `pBios.h`
* **Core Responsibility**: To act as the main operating loop and state machine for the pBios environment. It takes exclusive, blocking control of the system hardware, initializes its own minimal set of UI components, and manages navigation between the various pBios diagnostic screens.
* **Key Logic & Ideology**:
    * **System Takeover**: The core ideology of pBios is to be a "safe mode." When active, it prevents the main FreeRTOS tasks from running. The `pBios::run()` function is a simple, blocking `while` loop that takes direct control of the program flow. This ensures that diagnostics can be run on hardware without any interference from the main application's complex, multi-threaded logic.
    * **Hierarchical Menu System**: It implements a classic, pointer-based menu system. An array of `pBiosScreen*` objects defines the available diagnostic screens. The `pBios` loop handles user input (encoder/buttons) to cycle through this array, effectively changing the `_currentScreen` pointer to navigate the UI.
    * **Energy Consumption Logging**: Before shutting down or exiting, pBios writes a timestamp to a log file on the SD card (`/pBios.log`). This is a crucial piece of the **PowerMonitor's** logic, allowing it to calculate and reconcile the energy consumed during the pBios session on the next normal boot.
* **Architectural Flaws**:
    * **Blocking & Direct Access**: The entire pBios system is built on blocking calls and direct hardware manipulation (e.g., `displayManager.selectOLED()`, `ledManager.setState()`). While this is a *deliberate and necessary choice* for a low-level diagnostic tool to ensure stability, it represents an architectural pattern that is correctly avoided in the main, multi-threaded application.
    * **Code Duplication**: To remain self-contained, pBios sometimes duplicates logic found elsewhere. For example, its menu drawing routines are similar to, but separate from, the main `UIEngine`. This is a trade-off for creating a truly isolated recovery environment.

#### **Legacy Component:** `src/pBios/screens/*.cpp` (e.g., `pBios_SystemInfoScreen.cpp`, `pBios_SDCardScreen.cpp`)
* **Core Responsibility**: Each screen file is responsible for a single, specific diagnostic or utility function. For example, `pBios_SDCardScreen` handles formatting the SD card, while `pBios_SystemInfoScreen` displays firmware versions and hardware status.
* **Key Logic & Ideology**:
    * **Single-Purpose Tools**: Each screen is a "tool" in the pBios "toolbox." It has three main functions: `enter()`, `update()`, and `exit()`. The `update()` function contains the screen's main loop, where it handles user input and calls the appropriate manager functions to perform its task (e.g., calling `storageEngine.formatSDCard()`).
    * **Direct Manager Interaction**: Unlike screens in the main application that might interact with data through abstract global structs, pBios screens call manager functions *directly* and often in a blocking manner. For example, the SD card screen directly invokes the `StorageEngine`'s formatting routine and waits for it to complete before giving control back to the user.
* **Architectural Flaws**:
    * **Tight Coupling**: These screens are tightly coupled to the managers (`StorageEngine`, `DisplayManager`, etc.) and the `UIEngine`. This is by design, but it makes them non-reusable outside of the pBios context. It underscores the philosophical separation between the main application's loosely-coupled architecture and the pBios's tightly-coupled, direct-control approach.

***

### Signal Processing Filters

These are the fundamental, reusable components used by the `SensorProcessor` to turn noisy, raw sensor readings into stable, reliable data. They are the first and most important step in the data processing pipeline.

#### **Legacy Component:** `src/modules/filters/PI_Filter.cpp` & `PI_Filter.h`
* **Core Responsibility**: To implement a Proportional-Integral (PI) controller combined with a median pre-filter. Its primary job is to smooth a noisy signal, quickly settle on a stable value, and eliminate the steady-state error that a simple filter might leave behind.
* **Key Logic & Ideology (The "Secret Sauce")**:
    * **Hybrid Filtering**: The "secret sauce" is the combination of two filter types. It first passes the raw input through a `MedianFilter` (see below) to aggressively reject any large, sudden noise spikes.
    * **PI Control Loop**: The cleaned signal is then fed into a classic PI controller.
        * The **Proportional** term (`Kp * error`) provides a fast response, quickly moving the output towards the target.
        * The **Integral** term (`Ki * integral`) slowly accumulates the past error. This is the key to accuracy, as it continues to push the output until the error is truly zero, something a proportional-only filter cannot guarantee.
    * **Anti-Windup Mechanism**: The filter is intelligent enough to know when its output is saturated (i.e., at its maximum or minimum limit). When this happens, it stops accumulating the integral term (`_is_saturated` flag). This prevents the integral from growing to a massive, useless value ("integral windup"), which would cause a long recovery delay once the signal returns to the normal range.
* **Architectural Flaws**:
    * **Parameter Tuning Required**: The filter's performance is critically dependent on the `Kp` and `Ki` constants. These values were found through empirical tuning for this specific application (pH/EC sensors). It is not a "smart" or adaptive filter; it's a highly-tuned tool that requires external knowledge to configure correctly for a new application.

#### **Legacy Component:** `src/modules/filters/MedianFilter.cpp` & `MedianFilter.h`
* **Core Responsibility**: To provide highly effective, non-linear noise rejection. Its sole purpose is to remove outlier data points (spikes) from a signal stream before they can corrupt downstream calculations like averages or PI controllers.
* **Key Logic & Ideology**:
    * **Statistical Rejection**: Unlike a simple averaging filter which gets skewed by a single bad reading, a median filter is statistical. It maintains a "window" (a circular buffer) of the last N readings.
    * **Sort and Select**: When an output value is requested, the filter sorts its entire window of data and returns the middle value (the median). A sudden spike in the input will simply become the highest or lowest value in the window and will be ignored unless the spike persists for more than half the window's duration. This makes it exceptionally robust against transient electrical noise.
* **Architectural Flaws**:
    * **Computational Cost**: The primary drawback is its computational cost. Sorting the window on every single reading can be CPU-intensive, especially with large window sizes. This is a classic trade-off of computational expense for signal quality.
    * **Fixed Max Size**: The `MAX_WINDOW_SIZE` is a compile-time constant. This means the maximum filter window cannot be changed at runtime, limiting its flexibility without recompiling the firmware.
