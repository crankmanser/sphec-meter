// File Path: /lib/ADS1118/src/ADS1118.h

#ifndef ADS1118_h
#define ADS1118_h

#include "Arduino.h"
#include <SPI.h>
#include <stdint.h>

union Config {
	struct {					
		uint8_t reserved:1;
		uint8_t noOperation:2;
		uint8_t pullUp:1;
		uint8_t sensorMode:1;
		uint8_t rate:3;
		uint8_t operatingMode:1;
		uint8_t pga:3;
		uint8_t mux:3;
		uint8_t singleStart:1;
	} bits;
	uint16_t word;
	struct {
		uint8_t lsb;
		uint8_t msb;
	} byte;
};

class ADS1118 {
    public:
        void begin();
#if defined(__AVR__)
        ADS1118(uint8_t io_pin_cs);
#elif defined(ESP32)
        ADS1118(uint8_t io_pin_cs, SPIClass *spi = &SPI);
	void begin(uint8_t sclk, uint8_t miso, uint8_t mosi);
#endif
	double getTemperature();
        uint16_t getADCValue(uint8_t inputs);
	bool getADCValueNoWait(uint8_t pin_drdy, uint16_t &value);
	bool getMilliVoltsNoWait(uint8_t pin_drdy, double &volts);
        double getMilliVolts(uint8_t inputs);
	double getMilliVolts();
        void decodeConfigRegister(union Config configRegister);
	void setSamplingRate(uint8_t samplingRate);
	void setFullScaleRange(uint8_t fsr);
	void setContinuousMode();
	void setSingleShotMode();
	void disablePullup();
	void enablePullup();
	void setInputSelected(uint8_t input);

    // --- All constants are now static ---
	static const uint8_t DIFF_0_1 	  = 0b000;
	static const uint8_t DIFF_0_3 	  = 0b001;
	static const uint8_t DIFF_1_3 	  = 0b010;
    static const uint8_t DIFF_2_3 	  = 0b011;
    static const uint8_t AIN_0 	  = 0b100;
    static const uint8_t AIN_1		  = 0b101;
    static const uint8_t AIN_2 	  = 0b110;
    static const uint8_t AIN_3 	  = 0b111;
    union Config configRegister;

    static const uint32_t SCLK       = 2000000;
	static const uint8_t START_NOW   = 1;
	static const uint8_t ADC_MODE    = 0;
	static const uint8_t TEMP_MODE   = 1;
	static const uint8_t CONTINUOUS  = 0;
	static const uint8_t SINGLE_SHOT = 1;
	static const uint8_t DOUT_PULLUP  = 1;
	static const uint8_t DOUT_NO_PULLUP   = 0;
	static const uint8_t VALID_CFG   = 0b01;
	static const uint8_t NO_VALID_CFG= 0b00;
	static const uint8_t RESERVED    = 1;
    static const uint8_t FSR_6144    = 0b000;
    static const uint8_t FSR_4096    = 0b001;
    static const uint8_t FSR_2048    = 0b010;
    static const uint8_t FSR_1024    = 0b011;
    static const uint8_t FSR_0512    = 0b100;
    static const uint8_t FSR_0256    = 0b111;
    static const uint8_t RATE_8SPS   = 0b000;
    static const uint8_t RATE_16SPS  = 0b001;
    static const uint8_t RATE_32SPS  = 0b010;
    static const uint8_t RATE_64SPS  = 0b011;
    static const uint8_t RATE_128SPS = 0b100;
    static const uint8_t RATE_250SPS = 0b101;
    static const uint8_t RATE_475SPS = 0b110;
    static const uint8_t RATE_860SPS = 0b111;
		
private:
#if defined(ESP32)
	SPIClass *pSpi;
#endif  
	uint8_t lastSensorMode=3;
    uint8_t cs;
	const float pgaFSR[8] = {6.144, 4.096, 2.048, 1.024, 0.512, 0.256, 0.256, 0.256};
	const uint8_t CONV_TIME[8]={125, 63, 32, 16, 8, 4, 3, 2};
};

#endif