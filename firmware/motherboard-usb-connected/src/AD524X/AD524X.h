#ifndef AD524X_H_
#define AD524X_H_

#define AD_DEVICE_ADDRESS_1M      0x2C
#define AD_DEVICE_ADDRESS_100K    0x2E

#define AD524X_LIB_VERSION  (F("0.5.1"))
#define AD524X_OK           0
#define AD524X_ERROR        100
#define AD524X_MIDPOINT     127

#define AD524X_RDAC0        0x00
#define AD524X_RDAC1        0x80
#define AD524X_RESET        0x40
#define AD524X_SHUTDOWN     0x20
#define AD524X_O1_HIGH      0x10
#define AD524X_O2_HIGH      0x08

#include <stdint.h>
#include <stddef.h>

#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>

#include "../twowire/twowire.h"

uint8_t AD524X_1M_write(uint8_t value);
uint8_t AD524X_100K_write(uint8_t value);


/**************************************************/
/*                 OTHER functions                */
/**************************************************/


uint16_t AD524X_read_register(uint8_t dev_addr, uint8_t regaddr);
uint8_t AD524X_write_register(uint8_t dev_addr, uint8_t regaddr, uint8_t regval);



#endif