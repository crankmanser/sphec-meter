// File Path: /src/main.cpp

// ... (includes and global objects are unchanged) ...
#ifndef PIO_UNIT_TESTING
#include <Arduino.h>
#include "ProjectConfig.h"
#include "DebugConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include <FaultHandler.h>
#include <ConfigManager.h>
#include <DisplayManager.h>
#include <AdcManager.h>
#include <SdManager.h>
#include <INA219_Driver.h>
#include <PowerMonitor.h>
#include <FilterManager.h>
#include <CalibrationManager.h>
#include <TempManager.h>

FaultHandler faultHandler;
ConfigManager configManager;
DisplayManager displayManager;
AdcManager adcManager;
SdManager sdManager;
INA219_Driver ina219Driver;
PowerMonitor powerMonitor;
TempManager tempManager;
FilterManager phFilter;
FilterManager ecFilter;
FilterManager v3_3_Filter;
FilterManager v5_0_Filter;
CalibrationManager phCalManager;
CalibrationManager ecCalManager;
SPIClass* vspi = nullptr;
SemaphoreHandle_t spiMutex = nullptr;
portMUX_TYPE sharedDataMux = portMUX_INITIALIZER_UNLOCKED;
volatile double raw_ph_voltage = 0.0;
volatile double filtered_ph_voltage = 0.0;
double final_ph_value = 0.0;
double final_ec_value = 0.0;
double final_probe_temp = 0.0;
double final_ambient_temp = 0.0;
double final_humidity = 0.0;
double final_v3_3_bus = 0.0;
double final_v5_0_bus = 0.0;

void i2cTask(void* pvParameters);
void oneWireTask(void* pvParameters);
void sensorTask(void* pvParameters);
void telemetryTask(void* pvParameters);
void hardwareCalTask(void* pvParameters);
void softwareCalTask(void* pvParameters);
String getSerialInput();

void setup() {
    Serial.begin(115200);
    delay(1000);
    LOG_BOOT("SpHEC Meter v2.9.1 Booting...");

    spiMutex = xSemaphoreCreateMutex();
    vspi = new SPIClass(VSPI);
    vspi->begin(VSPI_SCK_PIN, VSPI_MISO_PIN, VSPI_MOSI_PIN);
    faultHandler.begin();
    displayManager.begin(faultHandler);
    displayManager.showBootScreen();
    adcManager.begin(faultHandler, vspi, spiMutex, SD_CS_PIN);
    sdManager.begin(faultHandler, vspi, spiMutex, SD_CS_PIN, ADC1_CS_PIN, ADC2_CS_PIN);
    configManager.begin(faultHandler);
    phFilter.begin(faultHandler, configManager);
    ecFilter.begin(faultHandler, configManager);
    v3_3_Filter.begin(faultHandler, configManager);
    v5_0_Filter.begin(faultHandler, configManager);
    phCalManager.begin(faultHandler);
    ecCalManager.begin(faultHandler);
    tempManager.begin(faultHandler);
    ina219Driver.begin(faultHandler);
    powerMonitor.begin(faultHandler, ina219Driver, sdManager);
    StaticJsonDocument<512> phCalDoc;
    if (sdManager.loadJson("/ph_cal.json", phCalDoc)) {
        phCalManager.deserializeModel(phCalManager.getMutableCurrentModel(), phCalDoc);
    }
    StaticJsonDocument<512> ecCalDoc;
    if (sdManager.loadJson("/ec_cal.json", ecCalDoc)) {
        ecCalManager.deserializeModel(ecCalManager.getMutableCurrentModel(), ecCalDoc);
    }
    xTaskCreate(i2cTask, "i2cTask", 2048, NULL, TASK_PRIORITY_LOW, NULL);
    xTaskCreate(oneWireTask, "oneWireTask", 2048, NULL, TASK_PRIORITY_LOW, NULL);
    xTaskCreate(sensorTask, "sensorTask", 4096, NULL, TASK_PRIORITY_NORMAL, NULL);
    xTaskCreate(telemetryTask, "telemetryTask", 4096, NULL, TASK_PRIORITY_LOW, NULL);
    xTaskCreate(hardwareCalTask, "hardwareCalTask", 4096, NULL, TASK_PRIORITY_LOW, NULL);
    xTaskCreate(softwareCalTask, "softwareCalTask", 4096, NULL, TASK_PRIORITY_LOW, NULL);
    LOG_BOOT("Initialization Complete.");
    adcManager.setProbeState(0, ProbeState::ACTIVE);
    adcManager.setProbeState(1, ProbeState::ACTIVE);
}
// ... (loop and other tasks are mostly unchanged) ...
void loop() { vTaskSuspend(NULL); }
String getSerialInput() {
    String input = ""; char c;
    while (true) {
        if (Serial.available()) {
            c = Serial.read();
            if (c == '\n' || c == '\r') {
                while(Serial.available() && (Serial.peek() == '\n' || Serial.peek() == '\r')) { Serial.read(); }
                return input;
            }
            input += c;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
void i2cTask(void* pvParameters) { for (;;) { powerMonitor.update(); vTaskDelay(pdMS_TO_TICKS(100)); } }
void oneWireTask(void* pvParameters) { for (;;) { tempManager.update(); vTaskDelay(pdMS_TO_TICKS(2000)); } }
void sensorTask(void* pvParameters) { 
    for (;;) {
        double raw_ph_v = adcManager.getVoltage(0, ADS1118::DIFF_0_1);
        double temp_filtered_ph_v = phFilter.process(raw_ph_v);
        taskENTER_CRITICAL(&sharedDataMux);
        raw_ph_voltage = raw_ph_v;
        filtered_ph_voltage = temp_filtered_ph_v;
        taskEXIT_CRITICAL(&sharedDataMux);
        double calibrated_ph = phCalManager.getCalibratedValue(temp_filtered_ph_v);
        final_ph_value = phCalManager.getCompensatedValue(calibrated_ph, tempManager.getProbeTemp(), false);
        double raw_ec_v = adcManager.getVoltage(1, ADS1118::DIFF_0_1);
        double filtered_ec_v = ecFilter.process(raw_ec_v);
        double calibrated_ec = ecCalManager.getCalibratedValue(filtered_ec_v);
        final_ec_value = ecCalManager.getCompensatedValue(calibrated_ec, tempManager.getProbeTemp(), true);
        double raw_v3_3 = adcManager.getVoltage(0, ADS1118::AIN_2); 
        final_v3_3_bus = v3_3_Filter.process(raw_v3_3);
        double raw_v5_0 = adcManager.getVoltage(1, ADS1118::AIN_2);
        final_v5_0_bus = v5_0_Filter.process(raw_v5_0);
        final_probe_temp = tempManager.getProbeTemp();
        final_ambient_temp = tempManager.getAmbientTemp();
        final_humidity = tempManager.getHumidity();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
void telemetryTask(void* pvParameters) { /* ... same as before ... */ 
    for (;;) {
        double pH, ec, p_temp, a_temp, hum, v33, v50, r_ph, f_ph;
        taskENTER_CRITICAL(&sharedDataMux);
        pH = final_ph_value; ec = final_ec_value; p_temp = final_probe_temp; a_temp = final_ambient_temp;
        hum = final_humidity; v33 = final_v3_3_bus; v50 = final_v5_0_bus;
        r_ph = raw_ph_voltage; f_ph = filtered_ph_voltage;
        taskEXIT_CRITICAL(&sharedDataMux);
        Serial.println("\n[TELEMETRY] -----------------------------");
        Serial.printf("  Probes:\n");
        Serial.printf("    - pH:         %.2f (Raw: %.2f mV, Filtered: %.2f mV)\n", pH, r_ph, f_ph);
        Serial.printf("    - EC:         %.2f uS/cm\n", ec);
        Serial.printf("  Temperatures:\n");
        Serial.printf("    - Probe:      %.2f C\n", p_temp);
        Serial.printf("    - Ambient:    %.2f C\n", a_temp);
        Serial.printf("    - Humidity:   %.1f %%\n", hum);
        Serial.printf("  System Bus:\n");
        Serial.printf("    - 3.3V Bus:   %.2f mV\n", v33);
        Serial.printf("    - 5.0V Bus:   %.2f mV\n", v50);
        Serial.printf("  Power:\n");
        Serial.printf("    - SOC:        %.1f %%\n", powerMonitor.getSOC());
        Serial.printf("    - Current:    %.2f mA\n", powerMonitor.getCurrent());
        Serial.println("-----------------------------------------\n");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
void hardwareCalTask(void* pvParameters) { /* ... same as before ... */ 
    vTaskDelay(pdMS_TO_TICKS(5500));
    Serial.println("\n\n===== Hardware Calibration Wizard =====");
    Serial.println("Press 'h' to adjust the pH hardware offset (Po).");
    Serial.println("This should be done ONCE with a new sensor module.");
    String choice = getSerialInput();
    if (choice == "h") {
        Serial.println("\n--- Hardware pH Offset Adjustment ---");
        Serial.println("1. Place the pH probe in a neutral (pH 7.0) buffer solution.");
        Serial.println("2. Wait for the voltage reading below to stabilize.");
        Serial.println("3. Use a screwdriver to turn the Po pot on the PH-4502C board");
        Serial.println("   until the voltage is as close to 2500.00 mV as possible.");
        Serial.println("\nPress ENTER when you are finished to exit.");
        while(Serial.available() == 0) {
            double current_raw_voltage;
            taskENTER_CRITICAL(&sharedDataMux);
            current_raw_voltage = raw_ph_voltage;
            taskEXIT_CRITICAL(&sharedDataMux);
            Serial.printf("--> Current Raw Voltage: %.2f mV\r", current_raw_voltage);
            vTaskDelay(pdMS_TO_TICKS(200));
        }
        while (Serial.available() > 0) { Serial.read(); }
        Serial.println("\nHardware adjustment complete.");
    } else {
        Serial.println("Skipping hardware calibration.");
    }
    Serial.println("Hardware calibration task finished. Suspending task.");
    vTaskSuspend(NULL);
}

void softwareCalTask(void* pvParameters) {
    vTaskDelay(pdMS_TO_TICKS(5000));
    Serial.println("\n\n===== Software Calibration Wizard =====");
    Serial.println("Press 's' to start 3-point software pH calibration.");
    Serial.println("Any other key to skip.");
    String choice = getSerialInput();

    if (choice == "s") {
        Serial.println("Starting Software pH Calibration...");
        phCalManager.startNewCalibration();
        const CalibrationModel& previousPhModel = phCalManager.getCurrentModel();
        for (int i = 0; i < CALIBRATION_POINT_COUNT; ++i) {
            Serial.printf("\n--- Point %d ---\n", i + 1);
            Serial.print("Enter nominal pH value for this point (e.g., 4.01): ");
            String input = getSerialInput();
            double nominalValue = input.toFloat();
            Serial.printf("Place probe in pH %.2f buffer solution.\n", nominalValue);
            Serial.println("Press ENTER when ready to capture voltage...");
            getSerialInput();
            double capturedVoltage, capturedTemp;
            taskENTER_CRITICAL(&sharedDataMux);
            capturedVoltage = filtered_ph_voltage;
            capturedTemp = final_probe_temp;
            taskEXIT_CRITICAL(&sharedDataMux);
            phCalManager.addCalibrationPoint(capturedVoltage, nominalValue, capturedTemp);
            Serial.printf("Captured Point %d: %.2f mV @ %.2f C -> Nominal pH %.2f\n", i + 1, capturedVoltage, capturedTemp, nominalValue);
        }
        Serial.println("\nCalculating new model...");
        double quality = phCalManager.calculateNewModel(previousPhModel);
        if (quality > 0) {
            const CalibrationModel& newModel = phCalManager.getNewModel();
            Serial.println("\n--- Calibration Results ---");
            Serial.printf("  - Quality Score: %.1f%%\n", newModel.qualityScore);
            Serial.printf("  - Curve Drift:   %.2f%%\n", newModel.sensorDrift);
            Serial.printf("  - Zero-Point Offset: %.2f mV (from 2500mV)\n", newModel.neutralVoltage - 2500.0);
            Serial.printf("  - Zero-Point Drift:  %.2f mV (from last cal)\n", newModel.zeroPointDrift);

            phCalManager.acceptNewModel();
            StaticJsonDocument<512> docToSave;
            phCalManager.serializeModel(phCalManager.getCurrentModel(), docToSave);
            if (sdManager.saveJson("/ph_cal.json", docToSave)) {
                Serial.println("\nSUCCESS: New pH model saved. Please reboot.");
            } else {
                Serial.println("\nERROR: Failed to save model to SD card.");
            }
        } else {
            Serial.println("ERROR: Failed to calculate new model.");
        }
    } else {
        Serial.println("Skipping software calibration.");
    }
    Serial.println("Software calibration task finished. Suspending task.");
    vTaskSuspend(NULL);
}
#endif // PIO_UNIT_TESTING