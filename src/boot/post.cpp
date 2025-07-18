// src/boot/post.cpp
// MODIFIED FILE
#include "post.h"
#include "presentation/DisplayManager.h"
#include "hal/INA219_Driver.h"
// <<< FIX: Include the new manual driver header
#include "hal/TCA9548_Manual_Driver.h"
#include "DebugMacros.h"

// External declarations for required manager/driver pointers
extern DisplayManager* displayManager;
extern INA219_Driver* ina219;
// <<< FIX: Corrected extern declaration to use the new manual driver type
extern TCA9548_Manual_Driver* tca9548;

bool run_post() {
    LOG_MAIN("--- Starting Power-On Self-Test (POST) ---\n");
    bool pass = true;

    // Test 1: I2C Bus Devices
    LOG_MAIN("[POST] Checking I2C bus...");
    // <<< FIX: This check will now work with the correct type
    if (!tca9548) {
        LOG_MAIN(" FAIL (TCA9548 object is null)\n");
        pass = false;
    } else {
        LOG_MAIN(" PASS\n");
    }

    // Test 2: Display Initialization
    LOG_MAIN("[POST] Testing displays...");
    if (!displayManager) {
        LOG_MAIN(" FAIL (DisplayManager object is null)\n");
        pass = false;
    } else {
        // Flash screens to provide visual feedback
        displayManager->clearAll();
        displayManager->getDisplay(OLED_ID::OLED_TOP)->fillRect(0,0,128,64,SSD1306_WHITE);
        displayManager->getDisplay(OLED_ID::OLED_MIDDLE)->fillRect(0,0,128,64,SSD1306_WHITE);
        displayManager->getDisplay(OLED_ID::OLED_BOTTOM)->fillRect(0,0,128,64,SSD1306_WHITE);
        displayManager->displayAll();
        delay(200);
        displayManager->clearAll();
        displayManager->displayAll();
        delay(100);
        LOG_MAIN(" PASS\n");
    }

    LOG_MAIN("--- POST Complete. Result: %s ---\n", pass ? "PASS" : "FAIL");
    return pass;
}