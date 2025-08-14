// File Path: /lib/ADS1118/src/ADS1118.cpp
// MODIFIED FILE

#include "ADS1118.h"
#include "Arduino.h"

#if defined(__AVR__)
ADS1118::ADS1118(uint8_t io_pin_cs) {
    cs = io_pin_cs;
}
#elif defined(ESP32)
ADS1118::ADS1118(uint8_t io_pin_cs, SPIClass *spi) {
    cs = io_pin_cs;
    pSpi = spi;
}
#endif

#if defined(__AVR__)
void ADS1118::begin() {
    pinMode(cs, OUTPUT);
    digitalWrite(cs, HIGH);
    SPI.begin();
    SPI.beginTransaction(SPISettings(SCLK, MSBFIRST, SPI_MODE1));
    configRegister.bits={RESERVED, VALID_CFG, DOUT_PULLUP, ADC_MODE, RATE_8SPS, SINGLE_SHOT, FSR_0256, DIFF_0_1, START_NOW};
}
#elif defined(ESP32)
void ADS1118::begin() {
    pinMode(cs, OUTPUT);
    digitalWrite(cs, HIGH);
    // pSpi->begin(); // <<< CRITICAL FIX: DO NOT re-initialize the bus. The main app is responsible for this.
    configRegister.bits={RESERVED, VALID_CFG, DOUT_PULLUP, ADC_MODE, RATE_8SPS, SINGLE_SHOT, FSR_0256, DIFF_0_1, START_NOW};
}

void ADS1118::begin(uint8_t sclk, uint8_t miso, uint8_t mosi) {
    pinMode(cs, OUTPUT);
	digitalWrite(cs, HIGH);
    pSpi->begin(sclk, miso, mosi, cs);
	configRegister.bits={RESERVED, VALID_CFG, DOUT_PULLUP, ADC_MODE, RATE_8SPS, SINGLE_SHOT, FSR_0256, DIFF_0_1, START_NOW};
}

bool ADS1118::getADCValueNoWait(uint8_t pin_drdy, uint16_t &value) {
    byte dataMSB, dataLSB;
	pSpi->beginTransaction(SPISettings(SCLK, MSBFIRST, SPI_MODE1));
	digitalWrite(cs, LOW);
	if (digitalRead(pin_drdy)) {
		digitalWrite(cs, HIGH);
		pSpi->endTransaction();
		return false;
	}

	dataMSB = pSpi->transfer(configRegister.byte.msb);
	dataLSB = pSpi->transfer(configRegister.byte.lsb);
	digitalWrite(cs, HIGH);
	pSpi->endTransaction();

	value = (dataMSB << 8) | (dataLSB);
    return true;
}

bool ADS1118::getMilliVoltsNoWait(uint8_t pin_drdy, double &volts) {
    float fsr = pgaFSR[configRegister.bits.pga];
	uint16_t value;
	bool dataReady=getADCValueNoWait(pin_drdy, value);
	if (!dataReady) return false;
	if(value>=0x8000){
		value=((~value)+1);
		volts=((float)(value*fsr/32768)*-1);
	} else {
		volts=(float)(value*fsr/32768);
	}
    volts = volts*1000;
	return true;
}
#endif

uint16_t ADS1118::getADCValue(uint8_t inputs) {
    uint16_t value;
    byte dataMSB, dataLSB, configMSB, configLSB, count=0;
	if(lastSensorMode==ADC_MODE)
		count=1;
	else
	configRegister.bits.sensorMode=ADC_MODE;
	configRegister.bits.mux=inputs;
    do{
        // <<< DEFINITIVE FIX: ALL SPI TRANSACTION CONTROL IS REMOVED >>>
        // The higher-level AdcManager is now responsible for this, which
        // prevents nested transactions and resolves the bus lock-up.
	    digitalWrite(cs, LOW);

        dataMSB = pSpi->transfer(configRegister.byte.msb);
        dataLSB = pSpi->transfer(configRegister.byte.lsb);
        configMSB = pSpi->transfer(configRegister.byte.msb);
        configLSB = pSpi->transfer(configRegister.byte.lsb);

	    digitalWrite(cs, HIGH);

	    for(int i=0;i<CONV_TIME[configRegister.bits.rate];i++)
            delayMicroseconds(1000);
            count++;
	}while (count<=1);
	value = (dataMSB << 8) | (dataLSB);
    return value;
}

double ADS1118::getMilliVolts(uint8_t inputs) {
    float volts;
    float fsr = pgaFSR[configRegister.bits.pga];
    uint16_t value;
    value=getADCValue(inputs);
    if(value>=0x8000){
	value=((~value)+1);
	volts=((float)(value*fsr/32768)*-1);
    } else {
	volts=(float)(value*fsr/32768);
    }
    return volts*1000;
}

double ADS1118::getMilliVolts() {
    float volts;
    float fsr = pgaFSR[configRegister.bits.pga];
    uint16_t value;
    value=getADCValue(configRegister.bits.mux);
    if(value>=0x8000){
	value=((~value)+1);
	volts=((float)(value*fsr/32768)*-1);
    } else {
	volts=(float)(value*fsr/32768);
    }
    return volts*1000;
}

double ADS1118::getTemperature() {
    uint16_t convRegister;
    uint8_t dataMSB, dataLSB, configMSB, configLSB, count=0;
    if(lastSensorMode==TEMP_MODE)
        count=1;
    else
	configRegister.bits.sensorMode=TEMP_MODE;
    do{
        // <<< DEFINITIVE FIX: ALL SPI TRANSACTION CONTROL IS REMOVED >>>
        digitalWrite(cs, LOW);
        dataMSB = pSpi->transfer(configRegister.byte.msb);
        dataLSB = pSpi->transfer(configRegister.byte.lsb);
        configMSB = pSpi->transfer(configRegister.byte.msb);
        configLSB = pSpi->transfer(configRegister.byte.lsb);
        digitalWrite(cs, HIGH);

	    for(int i=0;i<CONV_TIME[configRegister.bits.rate];i++)
            delayMicroseconds(1000);
        count++;
    }while (count<=1);
    convRegister = ((dataMSB << 8) | (dataLSB))>>2;
    if((convRegister<<2) >= 0x8000){
        convRegister=((~convRegister)>>2)+1;
        return (double)(convRegister*0.03125*-1);
    }
    return (double)convRegister*0.03125;
}

void ADS1118::setSamplingRate(uint8_t samplingRate){
    configRegister.bits.rate=samplingRate;
}

void ADS1118::setFullScaleRange(uint8_t fsr){
    configRegister.bits.pga=fsr;
}

void ADS1118::setInputSelected(uint8_t input){
    configRegister.bits.mux=input;
}

void ADS1118::setContinuousMode(){
    configRegister.bits.operatingMode=CONTINUOUS;
}

void ADS1118::setSingleShotMode(){
    configRegister.bits.operatingMode=SINGLE_SHOT;
}

void ADS1118::disablePullup(){
    configRegister.bits.operatingMode=DOUT_NO_PULLUP;
}

void ADS1118::enablePullup(){
    configRegister.bits.operatingMode=DOUT_PULLUP;
}

void ADS1118::decodeConfigRegister(union Config configRegister){
    // This function is for debugging and not needed for operation
}