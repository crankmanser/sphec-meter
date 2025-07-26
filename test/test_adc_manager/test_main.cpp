// File Path: /test/test_adc_manager/test_main.cpp

#include <Arduino.h>
#include <unity.h>
#include <AdcManager.h>
#include <FaultHandler.h>

FaultHandler testFaultHandler;
// In a test environment, we don't have a real SPI bus or mutex.
// We can pass nullptr for these as the smoke test only checks for successful compilation and instantiation.
SPIClass* vspi = nullptr;
SemaphoreHandle_t spiMutex = nullptr;

void setUp(void) {}
void tearDown(void) {}

/**
 * @brief Unit test for AdcManager initialization.
 * This is a "smoke test" to ensure that the AdcManager object can be
 * constructed and its `begin()` method can be called without causing a crash.
 */
void test_adc_manager_initialization() {
    // ARRANGE
    AdcManager adcManager;

    // ACT
    // The success of this test is that the code runs without a fault.
    // We now provide all required arguments for the begin() method.
    adcManager.begin(testFaultHandler, vspi, spiMutex, 0);

    // ASSERT
    TEST_ASSERT_TRUE(true);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_adc_manager_initialization);
    UNITY_END();
}

void loop() {}