/******************************************************************************
 *
 * This is a library to control all peripherals 
 * connected with the power transmitter unit.
 * 
 * Designed by Jarne Van Mulders
 *
 ******************************************************************************/

#include "peripherals.h"

int16_t channel_voltage [ADS1115_NUMBER_OF_CHANNELS];
// int16_t channel_result [ADS1115_NUMBER_OF_CHANNELS];

void perihperals_init(){
    // Reset ADS1115 ADC device
    ads1115_reset();

    // Initialize ADS1115 ADC device
    // ads1115_init(); // single shot
    ads1115_init_continuous(); // continuous

    ads1115_set_compare_channels(ADS1115_COMP_0_GND);


    // Inititalize programmable load

}

int16_t peripherals_ads1115_read(int channel){
    int16_t voltage;
    //ads1115_set_compare_channels(channel);
    voltage = ads1115_get_result_mV(); // alternative: getResult_mV for Millivolt
    return voltage;
}

void peripherals_ads1115_read_all_channels(){
    for(uint8_t i = 0; i < ADS1115_NUMBER_OF_CHANNELS; i++){
        channel_voltage[i] = peripherals_ads1115_read_channel(i);
    }
    // peripherals_convert_readings();
}

void peripherals_ads1115_read_one_channel(uint8_t channel){
    channel_voltage[channel] = peripherals_ads1115_read_channel(channel);
    // peripherals_convert_readings();
}

int16_t peripherals_ads1115_read_channel(uint8_t channel){
  int32_t voltage = 0;
  ads1115_set_single_channel(channel);
  ads1115_start_single_measurement();
  while(ads1115_is_busy()){}
  voltage = ads1115_get_result_mV();
  return voltage;
}

// void peripherals_convert_readings(){
//     for(uint8_t i = 0; i < ADS1115_NUMBER_OF_CHANNELS; i++){
//         switch (i){
//             case 0: buffer_voltage_mv = channel_voltage[i];     break;
//             case 1: break; 
//             case 2: break; 
//             case 3: break; 
//             default: break;
//         }
//     }
// }

void peripherals_set_digital_potentiometer(uint8_t value){
    AD524X_write(value);
}