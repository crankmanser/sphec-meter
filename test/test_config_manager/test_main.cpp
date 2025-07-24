// File Path: /test/test_config_manager/test_main.cpp

#include <Arduino.h>
#include <unity.h>
#include <ConfigManager.h>
#include <FaultHandler.h>

// A global instance of FaultHandler for the test environment.
FaultHandler testFaultHandler;

void setUp(void) {
    // This function is called before each test.
}

void tearDown(void) {
    // This function is called after each test.
}

//
// --- TEST CASE ---
//
// Test that the ConfigManager can be successfully initialized.
//
void test_config_manager_initialization() {
    // ARRANGE: Create an instance of the ConfigManager.
    ConfigManager configManager;

    // ACT: Call the begin method, passing our test FaultHandler.
    bool success = configManager.begin(testFaultHandler);

    // ASSERT: Verify that the begin method returns true, indicating success.
    TEST_ASSERT_TRUE(success);
}

//
// --- TEST RUNNER ---
//
void setup() {
    delay(2000); // Wait for serial monitor to connect.

    UNITY_BEGIN();
    RUN_TEST(test_config_manager_initialization);
    UNITY_END();
}

void loop() {
    // Not used in test environment.
}