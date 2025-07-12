#include "ADS1118_Driver.h"
#include "DebugMacros.h"
#include "config/hardware_config.h"

// <<< MODIFIED: Corrected the data type to SemaphoreHandle_t
ADS1118_Driver::ADS1118_Driver(uint8_t cs_pin, uint8_t other_adc_cs_pin, uint8_t sd_cs_pin, SPIClass* spi, SemaphoreHandle_t spi_bus_mutex) :
    _spi(spi),
    _adc(cs_pin, spi),
    _cs_pin(cs_pin),
    _other_adc_cs_pin(other_adc_cs_pin),
    _sd_cs_pin(sd_cs_pin),
    _spi_bus_mutex(spi_bus_mutex)
{}

void ADS1118_Driver::begin() {
    _adc.begin();
    LOG_HAL("ADS1118 Driver on CS Pin %d initialized.\n", _cs_pin);
}

void ADS1118_Driver::deselectOtherSlaves() {
    digitalWrite(_other_adc_cs_pin, HIGH);
    digitalWrite(_sd_cs_pin, HIGH);
}

int16_t ADS1118_Driver::readDifferential_0_1() {
    // Take control of the SPI bus
    xSemaphoreTake(_spi_bus_mutex, portMAX_DELAY);
    _spi->beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE1));
    
    deselectOtherSlaves();
    _adc.setInputSelected(_adc.DIFF_0_1);
    _adc.setFullScaleRange(_adc.FSR_4096);
    // ... (rest of the function is the same)
    uint16_t raw_value = _adc.getADCValue(_adc.DIFF_0_1);

    LOG_HAL("ADC%d (Differential 0-1) Raw Value: %d\n", _cs_pin == ADC1_CS_PIN ? 1 : 2, raw_value);
    
    // Release the SPI bus
    _spi->endTransaction();
    xSemaphoreGive(_spi_bus_mutex);

    return static_cast<int16_t>(raw_value);
}

int16_t ADS1118_Driver::readSingleEnded_2() {
    xSemaphoreTake(_spi_bus_mutex, portMAX_DELAY);
    _spi->beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE1));
    
    deselectOtherSlaves();
    _adc.setInputSelected(_adc.AIN_2);
    // ... (rest of the function is the same)
    uint16_t raw_value = _adc.getADCValue(_adc.AIN_2);

    LOG_HAL("ADC%d (Single-Ended 2) Raw Value: %d\n", _cs_pin == ADC1_CS_PIN ? 1 : 2, raw_value);

    _spi->endTransaction();
    xSemaphoreGive(_spi_bus_mutex);

    return static_cast<int16_t>(raw_value);
}

int16_t ADS1118_Driver::readSingleEnded_3() {
    xSemaphoreTake(_spi_bus_mutex, portMAX_DELAY);
    _spi->beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE1));

    deselectOtherSlaves();
    _adc.setInputSelected(_adc.AIN_3);
    // ... (rest of the function is the same)
    uint16_t raw_value = _adc.getADCValue(_adc.AIN_3);

    LOG_HAL("ADC%d (Single-Ended 3) Raw Value: %d\n", _cs_pin == ADC1_CS_PIN ? 1 : 2, raw_value);

    _spi->endTransaction();
    xSemaphoreGive(_spi_bus_mutex);

    return static_cast<int16_t>(raw_value);
}