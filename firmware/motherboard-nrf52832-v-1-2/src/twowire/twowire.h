/******************************************************************************
 *
 * This is a library for twowire or I2C communication.
 * 
 * Designed by Jarne Van Mulders
 *
 ******************************************************************************/

#ifndef TWOWIRE_H_
#define TWOWIRE_H_

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>

uint8_t twowire_write(uint8_t devaddr);

uint8_t twowire_write_register(uint8_t devaddr, uint8_t regaddr);

uint8_t twowire_read_register_data(uint8_t devaddr, uint8_t regaddr, uint8_t *regval);
uint8_t twowire_write_register_byte(uint8_t devaddr, uint8_t regaddr, uint8_t regval);
uint8_t twowire_write_register_data(uint8_t devaddr, uint8_t regaddr, uint16_t regval);

#endif
