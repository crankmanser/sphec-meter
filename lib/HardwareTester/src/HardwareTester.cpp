// File Path: /lib/HardwareTester/src/HardwareTester.cpp
// MODIFIED FILE

#include "HardwareTester.h"
#include "ProjectConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

HardwareTester::HardwareTester() {}

void HardwareTester::runTests(
    SdManager& sdManager,
    DisplayManager& displayManager,
    AdcManager& adcManager,
    TempManager& tempManager,
    std::vector<TestResult>& results) 
{
    results.clear();
    
    testSdCard(sdManager, results);
    testOleds(displayManager, results);
    // The I2C test has been removed to ensure pBIOS stability.
    testAdcs(adcManager, results);
    testTempSensors(tempManager, results);
}

void HardwareTester::testSdCard(SdManager& sdManager, std::vector<TestResult>& results) {
    results.push_back({"SD Card", TestStatus::RUNNING});
    const char* test_file = "/hw_test.tmp";
    StaticJsonDocument<32> doc;
    doc["test"] = "ok";
    results.back().status = sdManager.saveJson(test_file, doc) ? TestStatus::PASS : TestStatus::FAIL;
    vTaskDelay(pdMS_TO_TICKS(250));
}

void HardwareTester::testOleds(DisplayManager& displayManager, std::vector<TestResult>& results) {
    uint8_t channels[] = {OLED3_TCA_CHANNEL, OLED2_TCA_CHANNEL, OLED1_TCA_CHANNEL};
    const char* names[] = {"OLED Top", "OLED Middle", "OLED Bottom"};
    for (int i = 0; i < 3; ++i) {
        results.push_back({names[i], TestStatus::RUNNING});
        Adafruit_SSD1306* display = displayManager.getDisplay(i);
        if (display) {
            displayManager.selectTCAChannel(channels[i]);
            display->clearDisplay();
            display->fillRect(0, 0, 128, 64, SSD1306_INVERSE);
            display->display();
            vTaskDelay(pdMS_TO_TICKS(200));
            display->clearDisplay();
            display->display();
            results.back().status = TestStatus::PASS;
        } else {
            results.back().status = TestStatus::FAIL;
        }
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

void HardwareTester::testAdcs(AdcManager& adcManager, std::vector<TestResult>& results) {
    const int NUM_SAMPLES = 5;

    results.push_back({"ADC 1 (3.3V)", TestStatus::RUNNING});
    double v3_3_sum = 0;
    for (int i = 0; i < NUM_SAMPLES; ++i) {
        v3_3_sum += adcManager.getVoltage(0, ADS1118::AIN_2);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    double v3_3_avg = v3_3_sum / NUM_SAMPLES;
    results.back().status = (v3_3_avg > 3000 && v3_3_avg < 3600) ? TestStatus::PASS : TestStatus::FAIL;
    vTaskDelay(pdMS_TO_TICKS(250));

    results.push_back({"ADC 2 (5.0V)", TestStatus::RUNNING});
    double v5_0_sum = 0;
    for (int i = 0; i < NUM_SAMPLES; ++i) {
        v5_0_sum += adcManager.getVoltage(1, ADS1118::AIN_2);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    double v5_0_avg = v5_0_sum / NUM_SAMPLES;
    results.back().status = (v5_0_avg > 4700 && v5_0_avg < 5300) ? TestStatus::PASS : TestStatus::FAIL;
    vTaskDelay(pdMS_TO_TICKS(250));
}

void HardwareTester::testTempSensors(TempManager& tempManager, std::vector<TestResult>& results) {
    results.push_back({"Temp Probe", TestStatus::RUNNING});
    float temp = tempManager.getProbeTemp();
    if (!isnan(temp) && temp > -50 && temp < 100) {
        results.back().status = TestStatus::PASS;
    } else {
        results.back().status = TestStatus::FAIL;
    }
    vTaskDelay(pdMS_TO_TICKS(250));
}