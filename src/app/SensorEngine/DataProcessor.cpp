// src/app/SensorEngine/DataProcessor.cpp
// MODIFIED FILE
#include "DataProcessor.h"
#include "DebugMacros.h"

// Include the full headers for the managers this module orchestrates.
#include "managers/sensor/RawSensorReader.h"
#include "managers/sensor/LiquidTempManager.h"
#include "managers/sensor/AmbientTempManager.h"
#include "managers/sensor/AmbientHumidityManager.h"
#include "managers/sensor/SensorProcessor.h"
#include "managers/sensor/LDRManager.h"
#include "managers/power/PowerManager.h"

DataProcessor::DataProcessor(
    RawSensorReader* rawSensorReader,
    LiquidTempManager* liquidTempManager,
    AmbientTempManager* ambientTempManager,
    AmbientHumidityManager* ambientHumidityManager,
    SensorProcessor* sensorProcessor,
    LDRManager* ldrManager,
    PowerManager* powerManager
) :
    _rawSensorReader(rawSensorReader),
    _liquidTempManager(liquidTempManager),
    _ambientTempManager(ambientTempManager),
    _ambientHumidityManager(ambientHumidityManager),
    _sensorProcessor(sensorProcessor),
    _ldrManager(ldrManager),
    _powerManager(powerManager)
{}

void DataProcessor::process() {
    LOG_TASK("DataProcessor: Processing sensor data...\n");

    // This sequence of calls is migrated directly from the old SensorTask.
    // It represents the complete data processing pipeline.
    _rawSensorReader->update();
    _liquidTempManager->update();
    _ambientTempManager->update();
    _ambientHumidityManager->update();
    _sensorProcessor->update();
    _ldrManager->update();
    _powerManager->update();

    LOG_TASK("DataProcessor: Processing complete.\n");
}