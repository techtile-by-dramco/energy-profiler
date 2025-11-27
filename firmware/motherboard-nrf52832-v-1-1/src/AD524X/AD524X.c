
#include "AD524X.h"


uint8_t AD524X_write(uint8_t value){
  return AD524X_write_register(AD524X_RDAC0, value);
}


/**************************************************/
/*                 OTHER functions                */
/**************************************************/


uint16_t AD524X_read_register(uint8_t regaddr){
    uint8_t read_buffer [2];
    twowire_read_register_data(AD_DEVICE_ADDRESS, regaddr, read_buffer);
    return (uint16_t)(read_buffer[0] << 8 | read_buffer[1]);
}

uint8_t AD524X_write_register(uint8_t regaddr, uint8_t regval){
    return twowire_write_register_byte(AD_DEVICE_ADDRESS, regaddr, regval);
}