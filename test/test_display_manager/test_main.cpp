// File Path: /test/test_display_manager/test_main.cpp

#include <Arduino.h>
#include <unity.h>
#include <DisplayManager.h>
#include <FaultHandler.h>

FaultHandler testFaultHandler;

void setUp(void) {}
void tearDown(void) {}

/**
 * @brief Unit test for DisplayManager initialization.
 * * NOTE: This test does not and cannot verify that pixels turn on.
 * It is a "smoke test" to ensure that the DisplayManager object can be
 * constructed and its `begin()` method can be called without causing a crash.
 * This proves the code is syntactically correct and structurally sound,
 * which is the primary goal for hardware-level unit tests.
 */
void test_display_manager_initialization() {
    // ARRANGE
    DisplayManager displayManager;

    // ACT
    // We expect `begin()` to return false in a test environment where
    // the I2C hardware is not actually connected. The important part is
    // that this call does not crash the system.
    bool result = displayManager.begin(testFaultHandler);

    // ASSERT
    // In a test environment without real hardware, initialization will fail.
    // The success of this test is that the code runs without a fault.
    TEST_ASSERT_FALSE(result); 
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_display_manager_initialization);
    UNITY_END();
}

void loop() {}