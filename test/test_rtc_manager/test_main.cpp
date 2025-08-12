// File Path: /test/test_rtc_manager/test_main.cpp
// NEW FILE

#include <Arduino.h>
#include <unity.h>
#include <RtcManager.h>
#include <FaultHandler.h>

// Global test objects
FaultHandler testFaultHandler;
SemaphoreHandle_t testI2cMutex = nullptr;

void setUp(void) {
    // Create the mutex for the test environment
    if (testI2cMutex == nullptr) {
        testI2cMutex = xSemaphoreCreateMutex();
    }
}

void tearDown(void) {
    // No-op
}

/**
 * @brief Unit test for RtcManager initialization.
 * This is a "smoke test" to ensure that the RtcManager object can be
 * constructed and its `begin()` method can be called without causing a crash.
 * In a test environment without a physical I2C device, begin() is expected
 * to return false.
 */
void test_rtc_manager_initialization() {
    // ARRANGE
    RtcManager rtcManager;

    // ACT
    // In a test environment without real I2C hardware, this will fail gracefully.
    bool result = rtcManager.begin(testFaultHandler, testI2cMutex);

    // ASSERT
    // The important part is that the call does not crash the system.
    TEST_ASSERT_FALSE(result);
}

/**
 * @brief Tests that getTimestamp returns a default value when not initialized.
 */
void test_get_timestamp_uninitialized() {
    // ARRANGE
    RtcManager rtcManager;
    char buffer[20];

    // ACT
    rtcManager.getTimestamp(buffer, sizeof(buffer));

    // ASSERT
    TEST_ASSERT_EQUAL_STRING("00000000-000000", buffer);
}

// --- TEST RUNNER ---
void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_rtc_manager_initialization);
    RUN_TEST(test_get_timestamp_uninitialized);
    UNITY_END();
}

void loop() {}