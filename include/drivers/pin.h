/**
 * @file 
 * @brief 
 * @author Zhy
 * @version v1.0
 * @date 2017-06-01
 */
#ifndef DRIVERS_PIN_H_
#define DRIVERS_PIN_H_
#include "device.h"

struct PinDevice
{
    Device_t dev;
    int (*pinwrite)(struct PinDevice *pindev, uint16_t write_data);
    int (*pinconfig)(struct PinDevice *pindev);
    int (*pinread)(struct PinDevice *pindev);
};
int PinInstall(struct PinDevice *pindev, const char *name, int flags);
#endif
/* vim:set et ts=4 sts=4 sw=4 ft=c: */
