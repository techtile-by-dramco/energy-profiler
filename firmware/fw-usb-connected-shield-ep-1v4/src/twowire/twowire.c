/******************************************************************************
 *
 * This is a library for twowire or I2C communication.
 * 
 * Designed by Jarne Van Mulders
 *
 ******************************************************************************/

#include "twowire.h"

const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(i2c0));

uint8_t twowire_write(uint8_t devaddr){
    int ret;
    if (!device_is_ready(dev))
        return -ENODEV;
    ret = i2c_write(dev, NULL, 0, devaddr);
    if (ret)
        return ret;
    return 0;
}

uint8_t twowire_write_register(uint8_t devaddr, uint8_t regaddr){
    int ret;
    if (!device_is_ready(dev))
        return -ENODEV;
    uint8_t buf [1] = {regaddr};
    ret = i2c_write(dev, buf, 1, devaddr);
    if (ret)
        return ret;
    return 0;
}

uint8_t twowire_read_register_data(uint8_t devaddr, uint8_t regaddr, uint8_t *regval){
    int ret;
    if (!device_is_ready(dev)) 
        return -ENODEV;
    ret = i2c_write_read(dev, devaddr, &regaddr, 1, regval, 2);
    if (ret) 
        return ret;
    return 0;
}

uint8_t twowire_write_register_byte(uint8_t devaddr, uint8_t regaddr, uint8_t regval){
    int ret;
    if (!device_is_ready(dev))
        return -ENODEV;
    uint8_t buf [2] = {regaddr, regval};
    ret = i2c_write(dev, buf, 2, devaddr);
    if (ret)
        return ret;
    return 0;
}

uint8_t twowire_write_register_data(uint8_t devaddr, uint8_t regaddr, uint16_t regval){
    int ret;
    if (!device_is_ready(dev))
        return -ENODEV;
    uint8_t buf [3] = {regaddr, (uint8_t)(regval >> 8), (uint8_t)(regval)};
    ret = i2c_write(dev, buf, 3, devaddr);
    if (ret)
        return ret;
    return 0;
}