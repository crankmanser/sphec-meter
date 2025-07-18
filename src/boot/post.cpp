// src/boot/post.cpp
// MODIFIED FILE
#include "post.h"
#include "app/AppContext.h" // <<< ADDED
#include "presentation/DisplayManager.h"
#include "hal/TCA9548_Manual_Driver.h"
#include "DebugMacros.h"

bool run_post(AppContext* appContext) {
    LOG_MAIN("--- Starting Power-On Self-Test (POST) ---\n");
    bool pass = true;

    // Test 1: I2C Bus Devices
    LOG_MAIN("[POST] Checking I2C bus...");
    if (!appContext->tca) {
        LOG_MAIN(" FAIL (TCA9548 object is null)\n");
        pass = false;
    } else {
        LOG_MAIN(" PASS\n");
    }

    // Test 2: Display Initialization
    LOG_MAIN("[POST] Testing displays...");
    if (!appContext->displayManager) {
        LOG_MAIN(" FAIL (DisplayManager object is null)\n");
        pass = false;
    } else {
        // Flash screens to provide visual feedback
        appContext->displayManager->clearAll();
        appContext->displayManager->getDisplay(OLED_ID::OLED_TOP)->fillRect(0,0,128,64,SSD1306_WHITE);
        appContext->displayManager->getDisplay(OLED_ID::OLED_MIDDLE)->fillRect(0,0,128,64,SSD1306_WHITE);
        appContext->displayManager->getDisplay(OLED_ID::OLED_BOTTOM)->fillRect(0,0,128,64,SSD1306_WHITE);
        appContext->displayManager->displayAll();
        delay(200);
        appContext->displayManager->clearAll();
        appContext->displayManager->displayAll();
        delay(100);
        LOG_MAIN(" PASS\n");
    }

    LOG_MAIN("--- POST Complete. Result: %s ---\n", pass ? "PASS" : "FAIL");
    return pass;
}