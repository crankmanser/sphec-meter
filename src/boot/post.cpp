// src/boot/post.cpp
#include "post.h"
#include "presentation/DisplayManager.h"
#include "hal/INA219_Driver.h"
#include "hal/TCA9548_Wrapper.h"
#include "DebugMacros.h"

// External declarations for required manager/driver pointers
extern DisplayManager* displayManager;
extern INA219_Driver* ina219;
extern TCA9548_Wrapper* tca9548;

bool run_post() {
    LOG_MAIN("--- Starting Power-On Self-Test (POST) ---\n");
    bool pass = true;

    // Test 1: I2C Bus Devices
    LOG_MAIN("[POST] Checking I2C bus...");
    // The wrapper's begin() checks for TCA connection.
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