/**
 * @file i2c.h
 * @brief i2c device object defines.
 * @author WANG Jun
 * @date 2017-11-03
 *
 * update:
 * 2017-11-03   WANG Jun    first version.
 */
#ifndef DRIVERS_I2C_H_
#define DRIVERS_I2C_H_

#include "frameworks.h"

#ifdef __cplusplus
extern "C" {
#endif

struct I2CMessage {
    uint16_t address;
    uint16_t length;
    uint8_t *buffer;
};

struct I2CDevice {
    Device_t dev;
    int master_transfer(struct I2CDevice *i2cdev, struct I2CMessage *msg, int count);
}

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_I2C_H_ */
/* vim:set et ts=4 sts=4 sw=4 ft=c: */
