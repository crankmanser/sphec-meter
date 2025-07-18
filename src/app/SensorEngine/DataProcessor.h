// src/app/SensorEngine/DataProcessor.h
// MODIFIED FILE
#pragma once

// Forward-declare all required manager classes to avoid circular dependencies
// and keep the header clean.
class RawSensorReader;
class LiquidTempManager;
class AmbientTempManager;
class AmbientHumidityManager;
class SensorProcessor;
class LDRManager;
class PowerManager;

/**
 * @class DataProcessor
 * @brief A helper module responsible for the data processing stage of the pipeline.
 *
 * This class orchestrates the various sensor and power managers to convert
 * raw sensor readings into final, processed scientific values. It is designed
 * to be called from the master SensorEngineTask.
 */
class DataProcessor {
public:
    /**
     * @brief Constructs the DataProcessor.
     * @param rawSensorReader Pointer to the raw sensor data acquisition module.
     * @param liquidTempManager Pointer to the liquid temperature processing manager.
     * @param ambientTempManager Pointer to the ambient temperature processing manager.
     * @param ambientHumidityManager Pointer to the ambient humidity processing manager.
     * @param sensorProcessor Pointer to the core pH/EC processing manager.
     * @param ldrManager Pointer to the light level processing manager.
     * @param powerManager Pointer to the power and battery management module.
     */
    DataProcessor(
        RawSensorReader* rawSensorReader,
        LiquidTempManager* liquidTempManager,
        AmbientTempManager* ambientTempManager,
        AmbientHumidityManager* ambientHumidityManager,
        SensorProcessor* sensorProcessor,
        LDRManager* ldrManager,
        PowerManager* powerManager
    );

    /**
     * @brief Executes one full cycle of data processing.
     *
     * This method calls the update() function on all relevant managers in the
     * correct sequence to populate the global processed data struct.
     */
    void process();

private:
    // Pointers to all required manager dependencies.
    RawSensorReader* _rawSensorReader;
    LiquidTempManager* _liquidTempManager;
    AmbientTempManager* _ambientTempManager;
    AmbientHumidityManager* _ambientHumidityManager;
    SensorProcessor* _sensorProcessor;
    LDRManager* _ldrManager;
    PowerManager* _powerManager;
};