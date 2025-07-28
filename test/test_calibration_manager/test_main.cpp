// File Path: /test/test_calibration_manager/test_main.cpp

#include <Arduino.h>
#include <unity.h>
#include <CalibrationManager.h>
#include <FaultHandler.h>

// --- Global Test Objects ---
FaultHandler testFaultHandler;
CalibrationManager calManager;

// --- Test Suite Setup & Teardown ---
void setUp(void) {
    // This is called before each test.
    calManager.begin(testFaultHandler);
    calManager.startNewCalibration();
}

void tearDown(void) {
    // This is called after each test.
}

// --- TEST CASES ---

void test_calibration_manager_initialization() {
    CalibrationManager localCalManager;
    bool success = localCalManager.begin(testFaultHandler);
    TEST_ASSERT_TRUE(success);
}

void test_quadratic_model_calculation() {
    // ARRANGE: y = 0.5x^2 + 2x + 5
    // --- FIX: Add temperature argument to all calls ---
    calManager.addCalibrationPoint(1.0, 7.5, 25.0);
    calManager.addCalibrationPoint(2.0, 11.0, 25.0);
    calManager.addCalibrationPoint(3.0, 15.5, 25.0);

    // ACT
    calManager.calculateNewModel(CalibrationModel());
    const CalibrationModel& newModel = calManager.getNewModel();

    // ASSERT
    TEST_ASSERT_TRUE(newModel.isCalibrated);
    TEST_ASSERT_FLOAT_WITHIN(1e-6, 0.5, newModel.coeff_a);
    TEST_ASSERT_FLOAT_WITHIN(1e-6, 2.0, newModel.coeff_b);
    TEST_ASSERT_FLOAT_WITHIN(1e-6, 5.0, newModel.coeff_c);
}

void test_get_calibrated_value() {
    // ARRANGE
    // --- FIX: Add temperature argument to all calls ---
    calManager.addCalibrationPoint(1.0, 7.5, 25.0);
    calManager.addCalibrationPoint(2.0, 11.0, 25.0);
    calManager.addCalibrationPoint(3.0, 15.5, 25.0);
    calManager.calculateNewModel(CalibrationModel());
    calManager.acceptNewModel(); 

    // ACT: Expected y = 0.5*(4^2) + 2*4 + 5 = 21.0
    double calibratedValue = calManager.getCalibratedValue(4.0);

    // ASSERT
    TEST_ASSERT_FLOAT_WITHIN(1e-6, 21.0, calibratedValue);
}

void test_kpi_calculation() {
    // ARRANGE: Create a "previous" model for comparison
    CalibrationModel prevModel;
    prevModel.isCalibrated = true;
    prevModel.coeff_a = 0.5;
    prevModel.coeff_b = 2.0; // Old slope
    prevModel.coeff_c = 5.0;

    // ARRANGE: Create new calibration points with a slightly different slope
    // y = 0.5x^2 + 2.2x + 5 (b changed from 2.0 to 2.2, a 10% change)
    // --- FIX: Add temperature argument to all calls ---
    calManager.addCalibrationPoint(1.0, 7.7, 25.0);
    calManager.addCalibrationPoint(2.0, 11.4, 25.0);
    calManager.addCalibrationPoint(3.0, 16.1, 25.0);

    // ACT: Calculate the new model, passing the previous one for comparison
    calManager.calculateNewModel(prevModel);
    const CalibrationModel& newModel = calManager.getNewModel();

    // ASSERT: Quality Score
    // R-squared should be 100 because the points are perfect. (100 * 0.6 = 60)
    // Slope score should be 90% because slope changed by 10%. (90 * 0.4 = 36)
    // Expected Quality Score = 60 + 36 = 96.0
    TEST_ASSERT_FLOAT_WITHIN(0.1, 96.0, newModel.qualityScore);

    // ASSERT: Sensor Drift
    TEST_ASSERT_GREATER_THAN(0.0, newModel.sensorDrift);
}


// --- TEST RUNNER ---
void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_calibration_manager_initialization);
    RUN_TEST(test_quadratic_model_calculation);
    RUN_TEST(test_get_calibrated_value);
    RUN_TEST(test_kpi_calculation);
    UNITY_END();
}

void loop() {}