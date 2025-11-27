/******************************************************************************
 *
 * This is a library for the ADS1115 A/D Converter
 *
 * You'll find several example sketches which should enable you to use the library.
 *
 * You are free to use it, change it or build on it. In case you like it, it would
 * be cool if you give it a star.
 *
 * If you find bugs, please inform me!
 *
 * Written by Wolfgang (Wolle) Ewald
 * https://wolles-elektronikkiste.de/en/ads1115-a-d-converter-with-amplifier (English)
 * https://wolles-elektronikkiste.de/ads1115 (German)
 *
 *
 ******************************************************************************/

#ifndef ADS1115_WE_H_
#define ADS1115_WE_H_

#include <stdint.h>
#include <stddef.h>

#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>

// #include <zephyr/device.h>
// #include <zephyr/devicetree.h>
// #include <zephyr/drivers/i2c.h>

#include "../twowire/twowire.h"

#define ADS_DEVICE_ADDRESS  0x48

#define ADS1115_NUMBER_OF_CHANNELS  4

typedef enum ADS1115_COMP_QUE {
    ADS1115_ASSERT_AFTER_1 = 0x0000,
    ADS1115_ASSERT_AFTER_2 = 0x0001,
    ADS1115_ASSERT_AFTER_4 = 0x0002,
    ADS1115_DISABLE_ALERT  = 0x0003
} compQue;

typedef enum ADS1115_LATCH {
    ADS1115_LATCH_DISABLED = 0x0000,
    ADS1115_LATCH_ENABLED  = 0x0004,
} latch;

typedef enum ADS1115_ALERT_POL {
    ADS1115_ACT_LOW  = 0x0000,
    ADS1115_ACT_HIGH = 0x0008
} alertPol;

typedef enum ADS1115_COMP_MODE{
    ADS1115_MAX_LIMIT = 0x0000,
    ADS1115_WINDOW    = 0x0010
} compMode;

typedef enum ADS1115_CONV_RATE{
    ADS1115_8_SPS   = 0x0000,
    ADS1115_16_SPS  = 0x0020,
    ADS1115_32_SPS  = 0x0040,
    ADS1115_64_SPS  = 0x0060,
    ADS1115_128_SPS = 0x0080,
    ADS1115_250_SPS = 0x00A0,
    ADS1115_475_SPS = 0x00C0,
    ADS1115_860_SPS = 0x00E0
} convRate;

typedef enum ADS1115_MEASURE_MODE{
    ADS1115_CONTINOUS  = 0x0000, // keeping misspelled enum for backwards compatibility.
    ADS1115_CONTINUOUS = 0x0000, 
    ADS1115_SINGLE     = 0x0100
} measureMode;

typedef enum ADS1115_RANGE{
    ADS1115_RANGE_6144  = 0x0000,
    ADS1115_RANGE_4096  = 0x0200,
    ADS1115_RANGE_2048  = 0x0400,
    ADS1115_RANGE_1024  = 0x0600,
    ADS1115_RANGE_0512  = 0x0800,
    ADS1115_RANGE_0256  = 0x0A00,
} range;

typedef enum ADS1115_MUX{
    ADS1115_COMP_0_1   = 0x0000,
    ADS1115_COMP_0_3   = 0x1000,
    ADS1115_COMP_1_3   = 0x2000,
    ADS1115_COMP_2_3   = 0x3000,
    ADS1115_COMP_0_GND = 0x4000,
    ADS1115_COMP_1_GND = 0x5000,
    ADS1115_COMP_2_GND = 0x6000,
    ADS1115_COMP_3_GND = 0x7000
} mux;
#define ADS1115_COMP_INC 0x1000     // increment to next channel

typedef enum ADS1115_STATUS_OR_START{
    ADS1115_BUSY          = 0x0000,
    ADS1115_START_ISREADY = 0x8000
} statusOrStart;


#define ADS1115_CONV_REG       0x00 // Conversion Register
#define ADS1115_CONFIG_REG     0x01 // Configuration Register
#define ADS1115_LO_THRESH_REG  0x02 // Low Threshold Register
#define ADS1115_HI_THRESH_REG  0x03 // High Threshold Register
#define ADS1115_RESET          0x06

//         /* other */
#define ADS1115_REG_FACTOR    32768
#define ADS1115_REG_RESET_VAL 0x8583


void ads1115_reset();
uint8_t ads1115_init();

uint8_t ads1115_init_continuous();

/* Set number of conversions after which the alert pin will be active
    * - or you can disable the alert
    *
    *  ADS1115_ASSERT_AFTER_1  -> after 1 conversion
    *  ADS1115_ASSERT_AFTER_2  -> after 2 conversions
    *  ADS1115_ASSERT_AFTER_4  -> after 4 conversions
    *  ADS1115_DISABLE_ALERT   -> disable comparator // alert pin (default)
    */
void ads1115_set_alert_pin_mode(compQue mode);

/* Enable or disable latch. If latch is enabled the alarm pin will be active until the
    * conversion register is read (getResult functions). If disabled the alarm pin will be
    * deactivated with next value within limits.
    *
    * ADS1115_LATCH_DISABLED (default)
    * ADS1115_LATCH_ENABLED
    */
void ads1115_set_alert_latch(latch latch);

/* Sets the alert pin polarity if active:
    *
    * Enable or disable latch. If latch is enabled the alarm pin will be active until the
    * conversion register is read (getResult functions). If disabled the alarm pin will be
    * deactivated with next value within limits.
    *
    * ADS1115_ACT_LOW  ->  active low (default)
    * ADS1115_ACT_HIGH ->  active high
    */
void ads1115_set_alert_pol(alertPol polarity);

/* Choose maximum limit or maximum and minimum alert limit (window)in Volt - alert pin will
    * be active when measured values are beyond the maximum limit or outside the window
    * Upper limit first: setAlertLimit_V(MODE, maximum, minimum)
    * In max limit mode the minimum value is the limit where the alert pin will be deactivated (if
    * not latched)
    *
    * ADS1115_MAX_LIMIT
    * ADS1115_WINDOW
    */
void ads1115_set_alert_mode_and_limit_V(compMode mode, float hithres, float lothres);

/* Set the conversion rate in SPS (samples per second)
    * Options should be self-explaining:
    *
    * ADS1115_8_SPS
    * ADS1115_16_SPS
    * ADS1115_32_SPS
    * ADS1115_64_SPS
    * ADS1115_128_SPS (default)
    * ADS1115_250_SPS
    * ADS1115_475_SPS
    * ADS1115_860_SPS
    */
void ads1115_set_conv_rate(convRate rate);

/* returns the conversion rate */
convRate ads1115_get_conv_rate();

/* Set continuous or single shot mode:
    *
    * ADS1115_CONTINUOUS  ->  continuous mode
    * ADS1115_SINGLE     ->  single shot mode (default)
    */
void ads1115_set_measure_mode(measureMode mode);

/* Set the voltage range of the ADC to adjust the gain:
    * Please note that you must not apply more than VDD + 0.3V to the input pins!
    *
    * ADS1115_RANGE_6144  ->  +/- 6144 mV
    * ADS1115_RANGE_4096  ->  +/- 4096 mV
    * ADS1115_RANGE_2048  ->  +/- 2048 mV (default)
    * ADS1115_RANGE_1024  ->  +/- 1024 mV
    * ADS1115_RANGE_0512  ->  +/- 512 mV
    * ADS1115_RANGE_0256  ->  +/- 256 mV
    */
void ads1115_set_voltage_range_mV(range range);

/* Set the voltage range automatically 
    * 1) changes into maximum range and continuous mode
    * 2) measures the voltage
    * 3) chooses the smallest range in which the measured voltage is <80% 
    *    of the range's maximum
    * 4) switches back to single shot mode if it was in this mode before
    *  
    * Please be aware that the procedure takes the the time needed for several conversions.
    * You should ony use it in case you expect stable or slowly changing voltages. 
    */
void ads1115_set_auto_range();

/* Set the automatic voltage range permanantly, but the range will only be changed if the 
    * measured value is outside 30 - 80% of the maximum value of the current range. 
    * Therefore this method is faster than setAutoRange(). 
    */
void ads1115_set_permanent_auto_range_mode(uint8_t autoMode);

/* Set the inputs to be compared
    *
    * ADS1115_COMP_0_1    ->  compares 0 with 1 (default)
    * ADS1115_COMP_0_3    ->  compares 0 with 3
    * ADS1115_COMP_1_3    ->  compares 1 with 3
    * ADS1115_COMP_2_3    ->  compares 2 with 3
    * ADS1115_COMP_0_GND  ->  compares 0 with GND
    * ADS1115_COMP_1_GND  ->  compares 1 with GND
    * ADS1115_COMP_2_GND  ->  compares 2 with GND
    * ADS1115_COMP_3_GND  ->  compares 3 with GND
*/
void ads1115_set_compare_channels(mux mux);

/* Set to channel (0-3) in single ended mode
    */
void ads1115_set_single_channel(uint8_t channel);

uint8_t ads1115_is_busy();
void ads1115_start_single_measurement();
float ads1115_get_result_V();
int16_t ads1115_get_result_mV();

/* Get the raw result from the conversion register: 
    * The conversion register contains the conversion result of the amplified (!)
    * voltage. This means the value depends on the voltage as well as on the 
    * voltage range. E.g. if the voltage range is 6144 mV (ADS1115_RANGE_6144), 
    * +32767 is 6144 mV; if the range is 4096 mV, +32767 is 4096 mV, and so on.  
    */
int16_t ads1115_get_raw_result();

/* Scaling of the result to a different range: 
    * The results in the conversion register are in a range of -32767 to +32767
    * You might want to receive the result in a different scale, e.g. -1023 to 1023.
    * For -1023 to 1023, and if you have chosen e.g. ADS1115_RANGE_4096, 0 Volt would 
    * give 0 as result and 4096 mV would give 1023. -4096 mV would give -1023.
    */
int16_t ads1115_get_result_with_range(int16_t min, int16_t max);

/* Scaling of the result to a different range plus scaling to a voltage range: 
    * You can use this variant if you also want to scale to a voltage range. E.g. in
    * in order to get results equivalent to an Arduino UNO (10 bit, 5000 mV range), you 
    * would choose getResultWithRange(-1023, 1023, 5000). A difference to the Arduino 
    * UNO is that you can measure negative voltages. 
    * You have to ensure that the voltage range you scale to is smaller than the 
    * measuring voltage range.
    */
int16_t ads1115_get_result_with_range_voltage(int16_t min, int16_t max, int16_t maxVoltage);

/* This function returns the voltage range ADS1115_RANGE_XXXX in Millivolt */
uint16_t ads1115_get_voltage_range_mV();

/* With this function the alert pin will be active, when a conversion is ready.
    * In order to deactivate, use the setAlertLimit_V function
    */
void ads1115_set_alert_pin_to_conversion_ready();
void ads1115_clear_alert();


void ads1115_delay_acc_to_rate(convRate cr);
int16_t ads1115_calc_limit(float rawLimit);

uint32_t map(uint32_t au32_IN, uint32_t au32_INmin, uint32_t au32_INmax, uint32_t au32_OUTmin, uint32_t au32_OUTmax);

uint16_t read_register(uint8_t regaddr);
// uint8_t read(uint8_t devaddr, uint8_t regaddr, uint8_t *regval);

void write_register(uint8_t regaddr, uint16_t regval);
// uint8_t write(uint8_t devaddr, uint8_t regaddr, uint16_t regval);


#endif

