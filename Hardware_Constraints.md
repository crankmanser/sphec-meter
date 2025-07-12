Hardware Constraints & Truths
This document lists the non-negotiable hardware truths that must be respected by all software layers. These constraints are derived from schematics, datasheets, and previous debugging efforts.

1. SPI Bus Instability
Constraint: The SPI bus is considered fundamentally untrustworthy and prone to entering an unknown state, particularly when multiple devices are present.

Software Mandate:

All SPI transactions must be protected by a mutex to ensure atomic access.

All drivers interacting with the SPI bus must be stateless.

For every transaction, the driver must perform a "brute-force" re-initialization of the bus settings (speed, mode) and explicitly manage all Chip Select (CS) lines.

A "Guardian" task will monitor the SPI bus mutex to detect and recover from hangs.

2. LED Configuration & Logic
Constraint: The device has one bicolor (Red/Green) LED at the top and one single Green LED at the bottom.

Hardware Truth: The LEDs are wired with common anode logic.

digitalWrite(pin, LOW) turns the LED ON.

digitalWrite(pin, HIGH) turns the LED OFF.

Software Mandate: All LED control logic must be encapsulated within the LEDManager, which will handle the inverted logic and PWM control internally. The "Amber" color is a software construct achieved by rapidly alternating the top Red and Green LEDs.

3. ADC Configuration
ADC1 (CS Pin 4):

Differential (A0-A1): pH Probe

Single-Ended (A2): 3.3V Bus Monitor

ADC2 (CS Pin 2):

Differential (A0-A1): EC Probe

Single-Ended (A2): 5V Bus Monitor

4. Probe Signal Conditioning
Constraint: Both the pH and EC sensor board outputs are passed through a 10k+10k voltage divider before reaching the ADC input.

Software Mandate: The SensorProcessor must account for this voltage divider by multiplying the raw voltage reading from the ADC by a factor of 2.

5. I2C Bus Configuration
Constraint: All I2C devices (RTC, OLEDs) are on a bus controlled by a TCA9548A multiplexer, the INA219 sits directly on the i2c bus.

Software Mandate: All I2C communication must first select the correct channel on the multiplexer via the TCA9548_Driver.