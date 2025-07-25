// File Path: /test/test_sd_manager/test_main.cpp

#include <Arduino.h>
#include <unity.h>
#include <SdManager.h>
#include <FaultHandler.h>
#include "ProjectConfig.h" // For SD_CS_PIN

FaultHandler testFaultHandler;

void setUp(void) {}
void tearDown(void) {}

/**
 * @brief Unit test for SdManager initialization.
 * This is a "smoke test" to ensure that the SdManager object can be
 * constructed and its `begin()` method can be called without causing a crash.
 * In a test environment without a physical SD card, begin() is expected to
 * return false. The test passes if the code simply runs without a fault.
 */
void test_sd_manager_initialization() {
    // ARRANGE
    SdManager sdManager;

    // ACT
    // In a test environment without a real SD card, initialization will fail.
    // The success of this test is that the code runs without a fault.
    bool result = sdManager.begin(testFaultHandler, SD_CS_PIN);

    // ASSERT
    TEST_ASSERT_FALSE(result);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_sd_manager_initialization);
    UNITY_END();
}

void loop() {}