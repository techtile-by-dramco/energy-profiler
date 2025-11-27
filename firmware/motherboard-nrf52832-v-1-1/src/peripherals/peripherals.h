/******************************************************************************
 *
 * This is a library to control all peripherals 
 * connected with the power transmitter unit.
 * 
 * Designed by Jarne Van Mulders
 *
 ******************************************************************************/

#ifndef PERIPHERALS_H_
#define PERIPHERALS_H_

#include "../ADS1115/ADS1115.h"
#include "../AD524X/AD524X.h"

void perihperals_init();

// Continuous test
int16_t peripherals_ads1115_read(int channel);

void peripherals_ads1115_read_all_channels();
void peripherals_ads1115_read_one_channel(uint8_t channel);

int16_t peripherals_ads1115_read_channel(uint8_t channel);

// void peripherals_convert_readings();

void peripherals_set_digital_potentiometer(uint8_t value);

#endif
