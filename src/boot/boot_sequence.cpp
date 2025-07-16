// src/boot/boot_sequence.cpp
// MODIFIED FILE
#include "boot_sequence.h"
#include "DebugMacros.h"
#include "config/hardware_config.h"
#include "app/common/SystemState.h"

// Include all required boot modules
#include "boot/init_i2c_devices.h"
#include "boot/init_hals.h"
#include "boot/init_tasks.h"
#include "boot/init_managers.h"
#include "boot/post.h"

// Include required managers and blocks for boot-time operations
#include "managers/storage/StorageManager.h"
#include "managers/io/ButtonManager.h"
#include "managers/io/EncoderManager.h"
#include "presentation/DisplayManager.h"
#include "presentation/blocks/BootBlock.h"

// Include application mode handlers
#include "app/modes/normal.h"
#include "app/modes/pbios.h"

#include "hal/ADS1118_Driver.h"
#include "hal/INA219_Driver.h"
#include "managers/diagnostics/NoiseAnalysisManager.h"


// External declarations for global objects instantiated in main.cpp
extern StorageManager* storageManager;
extern DisplayManager* displayManager;
extern ButtonManager* buttonManager;
extern EncoderManager* encoderManager;
extern BootMode g_boot_mode;
extern NoiseAnalysisManager* noiseAnalysisManager;
extern ADS1118_Driver* adc1;
extern ADS1118_Driver* adc2;
extern INA219_Driver* ina219;


void runBootSequence() {
    LOG_BOOT("--- SpHEC Meter Boot Sequence Initiated ---\n");

    LOG_BOOT("[1/6] Initializing I2C bus and devices...\n");
    if (!init_i2c_devices()) {
        LOG_BOOT("CRITICAL: I2C Device Initialization Failed. Halting.\n");
        while(true) { delay(1000); }
    }
    
    LOG_BOOT("[2/6] Initializing Storage Manager hardware...\n");
    if (!storageManager->begin()) {
        LOG_BOOT("CRITICAL: Storage Manager hardware failed. Halting.\n");
        while(true) { delay(1000); }
    }

    LOG_BOOT("[3/6] Detecting boot mode...\n");
    pinMode(BTN_MIDDLE_PIN, INPUT_PULLUP);
    pinMode(BTN_BOTTOM_PIN, INPUT_PULLUP);
    delay(10); 
    bool pBiosRequested = (digitalRead(BTN_MIDDLE_PIN) == LOW && digitalRead(BTN_BOTTOM_PIN) == LOW);
    g_boot_mode = pBiosRequested ? BootMode::DIAGNOSTICS : BootMode::NORMAL;
    LOG_BOOT("Result: %s Mode\n", pBiosRequested ? "DIAGNOSTICS" : "NORMAL");

    LOG_BOOT("[4/6] Checking system integrity...\n");
    if (!storageManager->checkAndClearShutdownFlag()) {
        LOG_BOOT("Improper shutdown detected. Running recovery and POST.\n");
        BootBlockProps props;
        props.title = "Recovery";
        props.message = "Checking files...";
        BootBlock::draw(displayManager, props);
        storageManager->requestRecovery(); 
        delay(1000); 
        run_post();
    } else {
        LOG_BOOT("Clean shutdown detected. Running POST.\n");
        run_post();
    }

    // --- FIX: Initialize fundamental input managers BEFORE starting tasks ---
    // This is the definitive fix for the EncoderTask crash. By calling begin() here,
    // we guarantee the encoder's RTOS queue exists before the EncoderTask is created.
    LOG_BOOT("[5/6] Initializing core input managers...\n");
    buttonManager->begin();
    encoderManager->begin();

    // --- PHASE 2: RTOS STARTUP ---
    LOG_BOOT("[6/6] Starting RTOS tasks for %s mode...\n", g_boot_mode == BootMode::NORMAL ? "NORMAL" : "DIAGNOSTICS");
    init_tasks();
    LOG_BOOT("Yielding to RTOS scheduler...\n");
    vTaskDelay(pdMS_TO_TICKS(10)); 

    // --- PHASE 3: APPLICATION HANDOFF ---
    LOG_BOOT("Handing off to main application...\n");
    if (g_boot_mode == BootMode::DIAGNOSTICS) {
        noiseAnalysisManager = new NoiseAnalysisManager(adc1, adc2, ina219, nullptr, nullptr, nullptr);
        run_pbios_mode();
    } else {
        run_normal_mode();
    }
}
