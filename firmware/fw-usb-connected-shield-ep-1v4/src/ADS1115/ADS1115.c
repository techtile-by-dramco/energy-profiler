/*****************************************
* This is a library for the ADS1115 A/D Converter
*
* You'll find an example which should enable you to use the library. 
*
* You are free to use it, change it or build on it. In case you like 
* it, it would be cool if you give it a star.
* 
* If you find bugs, please inform me!
* 
* Written by Wolfgang (Wolle) Ewald
* https://wolles-elektronikkiste.de/en/ads1115-a-d-converter-with-amplifier (English)
* https://wolles-elektronikkiste.de/ads1115 (German)
*
*******************************************/

#include "ADS1115.h"

// const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(i2c0));

uint16_t voltageRange;
measureMode deviceMeasureMode;
uint8_t i2cAddress;
uint8_t autoRangeMode;

void ads1115_reset(){
    // Write to register 06h to reset device
    twowire_write_register(ADS_DEVICE_ADDRESS, ADS1115_RESET);
}

uint8_t ads1115_init(){    
// #ifndef USE_TINY_WIRE_M_
//     _wire->beginTransmission(i2cAddress);
//     uint8_t success = _wire->endTransmission();
// #else
//     TinyWireM.beginTransmission(i2cAddress);
//     uint8_t success = TinyWireM.endTransmission();
// #endif
    uint8_t success = twowire_write(ADS_DEVICE_ADDRESS);
    if(success){
        return 0;
    }
    write_register(ADS1115_CONFIG_REG, ADS1115_REG_RESET_VAL);
    ads1115_set_voltage_range_mV(ADS1115_RANGE_2048);
    write_register(ADS1115_LO_THRESH_REG, 0x8000);
    write_register(ADS1115_HI_THRESH_REG, 0x7FFF);
    deviceMeasureMode = ADS1115_CONTINUOUS;//ADS1115_SINGLE;
    autoRangeMode = 0;
    return 1;
}

uint8_t ads1115_init_continuous(){
    uint8_t success = twowire_write(ADS_DEVICE_ADDRESS);
    if(success){
        return 0;
    }
    ads1115_set_voltage_range_mV(ADS1115_RANGE_6144);
    ads1115_set_compare_channels(ADS1115_COMP_0_GND);
    ads1115_set_measure_mode(ADS1115_CONTINUOUS);
    ads1115_set_conv_rate(ADS1115_860_SPS);
}

void ads1115_set_alert_pin_mode(compQue mode){
    uint16_t currentConfReg = read_register(ADS1115_CONFIG_REG);
    currentConfReg &= ~(0x8003);    
    currentConfReg |= mode;
    write_register(ADS1115_CONFIG_REG, currentConfReg);
}

void ads1115_set_alert_latch(latch latch){
    uint16_t currentConfReg = read_register(ADS1115_CONFIG_REG);
    currentConfReg &= ~(0x8004);    
    currentConfReg |= latch;
    write_register(ADS1115_CONFIG_REG, currentConfReg);
}

void ads1115_set_alert_pol(alertPol polarity){
    uint16_t currentConfReg = read_register(ADS1115_CONFIG_REG);
    currentConfReg &= ~(0x8008);    
    currentConfReg |= polarity;
    write_register(ADS1115_CONFIG_REG, currentConfReg);
}

void ads1115_set_alert_mode_and_limit_V(compMode mode, float hiThres, float loThres){
    uint16_t currentConfReg = read_register(ADS1115_CONFIG_REG);
    currentConfReg &= ~(0x8010);    
    currentConfReg |= mode;
    write_register(ADS1115_CONFIG_REG, currentConfReg);
    int16_t alertLimit = ads1115_calc_limit(hiThres);
    write_register(ADS1115_HI_THRESH_REG, alertLimit);
    alertLimit = ads1115_calc_limit(loThres);
    write_register(ADS1115_LO_THRESH_REG, alertLimit);
    
}

void ads1115_set_conv_rate(convRate rate){
    uint16_t currentConfReg = read_register(ADS1115_CONFIG_REG);
    currentConfReg &= ~(0x80E0);    
    currentConfReg |= rate;
    write_register(ADS1115_CONFIG_REG, currentConfReg);
}

convRate ads1115_get_conv_rate(){
    uint16_t currentConfReg = read_register(ADS1115_CONFIG_REG);
    return (convRate)(currentConfReg & 0xE0);
}
    
void ads1115_set_measure_mode(measureMode mode){
    uint16_t currentConfReg = read_register(ADS1115_CONFIG_REG);
    deviceMeasureMode = mode;
    currentConfReg &= ~(0x8100);    
    currentConfReg |= mode;
    write_register(ADS1115_CONFIG_REG, currentConfReg);
}

void ads1115_set_voltage_range_mV(range range){
    uint16_t currentVoltageRange = voltageRange;
    uint16_t currentConfReg = read_register(ADS1115_CONFIG_REG);
    uint16_t currentRange = (currentConfReg >> 9) & 7;
    uint16_t currentAlertPinMode = currentConfReg & 3;
    
    ads1115_set_measure_mode(ADS1115_SINGLE);
    
    switch(range){
        case ADS1115_RANGE_6144:
            voltageRange = 6144;
            break;
        case ADS1115_RANGE_4096:
            voltageRange = 4096;
            break;
        case ADS1115_RANGE_2048:
            voltageRange = 2048;
            break;
        case ADS1115_RANGE_1024:
            voltageRange = 1024;
            break;
        case ADS1115_RANGE_0512:
            voltageRange = 512;
            break;
        case ADS1115_RANGE_0256:
            voltageRange = 256;
            break;
    }
    
    if ((currentRange != range) && (currentAlertPinMode != ADS1115_DISABLE_ALERT)){
        int16_t alertLimit = read_register(ADS1115_HI_THRESH_REG);
        alertLimit = alertLimit * (currentVoltageRange * 1.0 / voltageRange);
        write_register(ADS1115_HI_THRESH_REG, alertLimit);
        
        alertLimit = read_register(ADS1115_LO_THRESH_REG);
        alertLimit = alertLimit * (currentVoltageRange * 1.0 / voltageRange);
        write_register(ADS1115_LO_THRESH_REG, alertLimit);
    }
    
    currentConfReg &= ~(0x8E00);    
    currentConfReg |= range;
    write_register(ADS1115_CONFIG_REG, currentConfReg);
    convRate rate = ads1115_get_conv_rate();
    ads1115_delay_acc_to_rate(rate);
}

void ads1115_set_auto_range(){
    uint16_t currentConfReg = read_register(ADS1115_CONFIG_REG);
    ads1115_set_voltage_range_mV(ADS1115_RANGE_6144);
    
    if(deviceMeasureMode == ADS1115_SINGLE){
        ads1115_set_measure_mode(ADS1115_CONTINUOUS);
        convRate rate = ads1115_get_conv_rate();
        ads1115_delay_acc_to_rate(rate);
    }
    
    int16_t rawResult = abs(read_register(ADS1115_CONV_REG));
    range optRange = ADS1115_RANGE_6144;
    
    if(rawResult < 1093){
        optRange = ADS1115_RANGE_0256;
    }
    else if(rawResult < 2185){
        optRange = ADS1115_RANGE_0512;
    }
    else if(rawResult < 4370){
        optRange = ADS1115_RANGE_1024;
    }
    else if(rawResult < 8738){
        optRange = ADS1115_RANGE_2048;
    }
    else if(rawResult < 17476){
        optRange = ADS1115_RANGE_4096;
    }
    
    write_register(ADS1115_CONFIG_REG, currentConfReg);
    ads1115_set_voltage_range_mV(optRange); 
}

void ads1115_set_permanent_auto_range_mode(uint8_t autoMode){
    if(autoMode){
        autoRangeMode = 1;
    }
    else{
        autoRangeMode = 0;
    }
}
        
void ads1115_delay_acc_to_rate(convRate cr){
    switch(cr){
        case ADS1115_8_SPS:
            k_msleep(130);
            break;
        case ADS1115_16_SPS:
            k_msleep(65);
            break;
        case ADS1115_32_SPS:
            k_msleep(32);
            break;
        case ADS1115_64_SPS:
            k_msleep(16);
            break;
        case ADS1115_128_SPS:
            k_msleep(8);
            break;
        case ADS1115_250_SPS:
            k_msleep(4);
            break;
        case ADS1115_475_SPS:
            k_msleep(3);
            break;
        case ADS1115_860_SPS:
            k_msleep(2);
            break;
    }
}
    

void ads1115_set_compare_channels(mux mux){
    uint16_t currentConfReg = read_register(ADS1115_CONFIG_REG);
    currentConfReg &= ~(0xF000);    
    currentConfReg |= (mux);
    write_register(ADS1115_CONFIG_REG, currentConfReg);
    
    if(!(currentConfReg & 0x0100)){  // => if not single shot mode
        convRate rate = ads1115_get_conv_rate();      
        ads1115_delay_acc_to_rate(rate);
        ads1115_delay_acc_to_rate(rate);               
    }       
}

void ads1115_set_single_channel(uint8_t channel) {
    if (channel >=  4)
        return;
    ads1115_set_compare_channels((mux)(ADS1115_COMP_0_GND + ADS1115_COMP_INC*channel));
}

uint8_t ads1115_is_busy(){
    uint16_t currentConfReg = read_register(ADS1115_CONFIG_REG);
    return (!(currentConfReg>>15) & 1);
}
    
void ads1115_start_single_measurement(){
    uint16_t currentConfReg = read_register(ADS1115_CONFIG_REG);
    currentConfReg |= (1 << 15);
    write_register(ADS1115_CONFIG_REG, currentConfReg);
}
    
float ads1115_get_result_V(){
    float result = ads1115_get_result_mV();
    result /= 1000;
    return result;  
}

int16_t ads1115_get_result_mV(){
    int16_t rawResult = ads1115_get_raw_result();
    // printk("raw: %d\n", rawResult);
    int16_t result = (int16_t)(rawResult * voltageRange / ADS1115_REG_FACTOR);
    // printk("calc: %d\n", result);
    return result;
}

int16_t ads1115_get_raw_result(){
    int16_t rawResult = read_register(ADS1115_CONV_REG);
    if(autoRangeMode){
        if((abs(rawResult) > 26214) && (voltageRange != 6144)){ // 80%
            ads1115_set_auto_range();
            rawResult = read_register(ADS1115_CONV_REG);
        }
        else if((abs(rawResult) < 9800) && (voltageRange != 256)){ //30%
            ads1115_set_auto_range();
            rawResult = read_register(ADS1115_CONV_REG);
        }
    }
    return rawResult;
}

int16_t ads1115_get_result_with_range(int16_t min, int16_t max){
    int16_t rawResult = ads1115_get_raw_result();
    int16_t result = map(rawResult, -32767, 32767, min, max);
    return result;
}

int16_t ads1115_get_result_with_range_voltage(int16_t min, int16_t max, int16_t maxVoltage){
    int16_t result = ads1115_get_result_with_range(min, max);
    result = (int16_t)((1.0 * result * voltageRange / maxVoltage) + 0.5);
    return result;
}

uint16_t ads1115_get_voltage_range_mV(){
    return voltageRange;
}

void ads1115_set_alert_pin_to_conversion_ready(){
    write_register(ADS1115_LO_THRESH_REG, (0<<15));
    write_register(ADS1115_HI_THRESH_REG, (1<<15));
}

void ads1115_clear_alert(){
    read_register(ADS1115_CONV_REG);
}

/**************************************************/
/*                 OTHER functions                */
/**************************************************/

int16_t ads1115_calc_limit(float rawLimit){
    int16_t limit = (int16_t)((rawLimit * ADS1115_REG_FACTOR / voltageRange)*1000);
    return limit;
}

uint32_t map(uint32_t au32_IN, uint32_t au32_INmin, uint32_t au32_INmax, uint32_t au32_OUTmin, uint32_t au32_OUTmax){
    return ((((au32_IN - au32_INmin)*(au32_OUTmax - au32_OUTmin))/(au32_INmax - au32_INmin)) + au32_OUTmin);
}

uint16_t read_register(uint8_t regaddr){
    uint8_t read_buffer [2];
    twowire_read_register_data(ADS_DEVICE_ADDRESS, regaddr, read_buffer);
    return (uint16_t)(read_buffer[0] << 8 | read_buffer[1]);
}

void write_register(uint8_t regaddr, uint16_t regval){
    twowire_write_register_data(ADS_DEVICE_ADDRESS, regaddr, regval);
}