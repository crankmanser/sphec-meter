// File Path: /test/test_fault_handler/test_main.cpp

#include <Arduino.h>
#include <unity.h> // The unit testing framework
#include <FaultHandler.h> // The class we are testing

//
// --- TEST SETUP AND TEARDOWN ---
//
// These functions are called by the test runner before and after each test.
// We can use them to set up and clean up our test environment.
//
void setUp(void) {
    // set up supporting mocks
}

void tearDown(void) {
    // clean up supporting mocks
}

//
// --- TEST CASE ---
//
// A simple test to verify that the FaultHandler object can be created.
// In a real-world scenario, we would mock the Serial port to verify
// that `trigger_fault` prints the correct messages, but for this first
// step, we are just confirming the basic structure.
//
void test_fault_handler_instantiation() {
    // ARRANGE: Create an instance of the FaultHandler.
    FaultHandler faultHandler;

    // ACT: Call the begin method.
    faultHandler.begin();

    // ASSERT: Check that the object is not null. This is a basic sanity check.
    // In C++, the object exists on the stack, so it can't be null.
    // The real test is that the code compiles and runs without crashing.
    // We use a Unity assertion to mark the test as passed.
    TEST_ASSERT_TRUE(true);
}


//
// --- TEST RUNNER ---
//
// This is the main entry point for the test suite.
//
void setup() {
    // Wait for a moment to let the serial monitor connect.
    delay(2000);

    UNITY_BEGIN(); // Start the test runner
    RUN_TEST(test_fault_handler_instantiation);
    UNITY_END(); // End the test runner
}

void loop() {
    // The loop is not used in the test environment.
}