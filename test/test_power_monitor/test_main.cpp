// File Path: /test/test_power_monitor/test_main.cpp
// MODIFIED FILE

#include <Arduino.h>
#include <unity.h>
#include <PowerMonitor.h>
#include <FaultHandler.h>
#include <INA219_Driver.h>
#include <I_StorageProvider.h>
#include <ArduinoJson.h>

// A global instance of FaultHandler for the test environment.
FaultHandler testFaultHandler;

// --- FIX: Add a null handle for the new mutex parameter ---
SemaphoreHandle_t testI2cMutex = nullptr;

// --- MOCK OBJECTS ---

class MockINA219 : public INA219_Driver {
public:
    float mockVoltage = 8.4f;
    float mockCurrent = 0.0f;

    // --- FIX: Update the signature to match the base class virtual method ---
    bool begin(FaultHandler& faultHandler, SemaphoreHandle_t i2cMutex) override { return true; }
    
    // These methods don't need the mutex as they return mock data.
    float getBusVoltage() override { return mockVoltage; }
    float getCurrent_mA() override { return mockCurrent; }
};

class MockStorage : public I_StorageProvider {
public:
    String storedJson;

    bool saveJson(const char* path, const JsonDocument& doc) override {
        storedJson = "";
        serializeJson(doc, storedJson);
        return true;
    }

    bool loadJson(const char* path, JsonDocument& doc) override {
        if (storedJson.length() == 0) return false;
        deserializeJson(doc, storedJson);
        return true;
    }
};

// --- TEST SETUP ---
PowerMonitor powerMonitor;
MockINA219 mockIna219;
MockStorage mockStorage;

void setUp(void) {
    // The mockIna219 doesn't need to be initialized with begin() for this test,
    // as the PowerMonitor itself will be tested.
    powerMonitor.begin(testFaultHandler, mockIna219, mockStorage);
}

void tearDown(void) {}

// --- TEST CASES ---

void test_power_monitor_initialization() {
    float soc = powerMonitor.getSOC();
    TEST_ASSERT_EQUAL_FLOAT(100.0f, soc);
}

/**
 * @brief Tests that the SOC decreases under a simulated discharge.
 *
 * This test simulates a 1A discharge for 10 seconds. It verifies that the
 * Coulomb Counting logic correctly subtracts energy and reports a lower SOC.
 */
void test_soc_decreases_on_discharge() {
    // ARRANGE: Set a discharge current and get the initial SOC.
    mockIna219.mockCurrent = -1000.0f; // 1A discharge
    float initial_soc = powerMonitor.getSOC();

    // ACT: Simulate 10 seconds of real time passing. The `update()` method's
    // internal timer will ensure its logic runs approximately 10 times.
    for (int i = 0; i < 10; i++) {
        powerMonitor.update();
        delay(1000);
    }
    
    float final_soc = powerMonitor.getSOC();

    // ASSERT: After discharging, the final SOC should be less than the initial SOC.
    // The drop will be small (~0.1%), but this proves the logic is working correctly.
    TEST_ASSERT_LESS_THAN(initial_soc, final_soc);
}

//
// --- TEST RUNNER ---
//
void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_power_monitor_initialization);
    RUN_TEST(test_soc_decreases_on_discharge);
    UNITY_END();
}

void loop() {}