/**
 * @file device.h
 * @brief device struct definitions, and include other devices' header file.
 * @author WANG Jun
 * @version v1.0
 * @date 2017-05-22
 *
 * update:
 * 2017-09-17   WANG Jun    Update defines.
 */
#ifndef FW_DEVICE_H_
#define FW_DEVICE_H_

#include "frameworks.h"

#ifdef INCLUDE_DEVICE

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup DeviceManagement
 * @{
 */

/**
 * Device flags
 */
#define DEVICE_FLAG_RO (1 << 0)
#define DEVICE_FLAG_WO (1 << 1)
#define DEVICE_FLAG_RW (DEVICE_FLAG_RO | DEVICE_FLAG_WO)
#define DEVICE_FLAG_RX_DMA (1 << 4)
#define DEVICE_FLAG_RX_INT (1 << 5)
#define DEVICE_FLAG_TX_DMA (1 << 6)
#define DEVICE_FLAG_TX_INT (1 << 7)
//#define DEVICE_FLAG_MULTIUSERS (1 << 8)
//#define DEVICE_FLAG_HOTPLUG (1 << 9)
#define DEVICE_FLAG_MASK 0x0FFF
/**
 * Device state
 */
#define DEVICE_STATE_OPENED (1 << 14)
#define DEVICE_STATE_INITIALIZED (1 << 15)
#define DEVICE_STATE_MASK 0xF000

/* device object check value */
#define DEVICE_CHECK_VALUE 0xA5A5A5A5

typedef struct DeviceStruct Device_t;

/**
 * Device object type, include device infomations and opearations.
 */
struct DeviceStruct
{
    uint32_t check_value; /*!< Known value to check object. */
    struct list_head node;
    const char *name;
    uint16_t flags; /*!< Device supported flags. */
    uint16_t state; /*!< Device used flags:[0..11] and current state:[12..15]. */

    int (*init)(Device_t *dev);
    int (*release)(Device_t *dev);
    int (*open)(Device_t *dev, int flags);
    int (*close)(Device_t *dev);
    int (*read)(Device_t *dev, void *buf, int size);
    int (*write)(Device_t *dev, const void *buf, int size);
    int (*ioctl)(Device_t *dev, int cmd, void *args);

    void *data;
};

Device_t *DeviceFindByName(const char *name);
int DeviceGetState(Device_t *dev);

int DeviceInstall(Device_t *dev, const char *name, int flags);
int DeviceRemove(Device_t *dev);

int DeviceInit(Device_t *dev);
int DeviceRelease(Device_t *dev);
int DeviceOpen(Device_t *dev, int flags);
int DeviceClose(Device_t *dev);
int DeviceRead(Device_t *dev, void *buf, int size);
int DeviceWrite(Device_t *dev, const void *buf, int size);
int DeviceIoctl(Device_t *dev, int cmd, void *args);

/** @} */

#ifdef __cplusplus
}
#endif

#ifdef INCLUDE_DEVICE_UART
#include "drivers/uart.h"
#endif

#ifdef INCLUDE_DEVICE_CAN
#include "drivers/can.h"
#endif

#ifdef INCLUDE_DEVICE_I2C
#include "drivers/i2c.h"
#endif

#ifdef INCLUDE_DEVICE_PIN
#include "drivers/pin.h"
#endif

#ifdef INCLUDE_DEVICE_SPI
#include "drivers/spi.h"
#endif

#ifdef INCLUDE_DEVICE_RTC
#include "drivers/rtc.h"
#endif

#ifdef INCLUDE_DEVICE_ADC
#include "drivers/adc.h"
#endif

#ifdef INCLUDE_DEVICE_HWTMR
#include "drivers/hwtmr.h"
#endif

#endif /* INCLUDE_DEVICE */
#endif /* FW_DEVICE_H_ */
/* vim:set et ts=4 sts=4 sw=4 ft=c: */
