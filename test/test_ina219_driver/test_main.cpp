// File Path: /test/test_ina219_driver/test_main.cpp

#include <Arduino.h>
#include <unity.h>
#include <INA219_Driver.h>
#include <FaultHandler.h>

// A global instance of FaultHandler for the test environment.
FaultHandler testFaultHandler;

void setUp(void) {
    // This function is called before each test.
}

void tearDown(void) {
    // This function is called after each test.
}

/**
 * @brief Unit test for INA219_Driver initialization.
 *
 * NOTE: This test does not and cannot verify communication with the physical sensor.
 * It is a "smoke test" to ensure that the INA219_Driver object can be
 * constructed and its `begin()` method can be called without causing a crash.
 * This proves the code is syntactically correct and structurally sound,
 * which is the primary goal for hardware-level unit tests.
 */
void test_ina219_driver_initialization() {
    // ARRANGE: Create an instance of the INA219_Driver.
    INA219_Driver ina219Driver;

    // ACT: Call the begin method. In a test environment without a real I2C
    // device, we expect this to return false. The important part is that
    // this call does not crash the system.
    bool result = ina219Driver.begin(testFaultHandler);

    // ASSERT: We expect initialization to fail gracefully.
    TEST_ASSERT_FALSE(result);
}

//
// --- TEST RUNNER ---
//
void setup() {
    delay(2000); // Wait for serial monitor to connect.

    UNITY_BEGIN();
    RUN_TEST(test_ina219_driver_initialization);
    UNITY_END();
}

void loop() {
    // Not used in test environment.
}