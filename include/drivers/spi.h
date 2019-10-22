/**
 * @file spi.h
 * @brief 
 * @author WANG Jun
 * @date 2017-10-19
 *
 * update:
 */
#ifndef DRIVERS_SPI_H_
#define DRIVERS_SPI_H_

#include "FreeRTOS.h"

#define SPI_CPHA 0x01 /* clock phase */
#define SPI_CPOL 0x02 /* clock polarity */
#define SPI_MODE_0 (0 | 0)
#define SPI_MODE_1 (0 | SPI_CPHA)
#define SPI_MODE_2 (SPI_CPOL | 0)
#define SPI_MODE_3 (SPI_CPOL | SPI_CPHA)

struct SpiConf
{
    uint16_t mode;
    uint16_t bits_per_width;
    uint32_t max_speed_hz;
};
typedef struct SpiConf SpiConf_t;

struct SpiMaster
{
    Device_t dev;
    SpiDevice_t *user;
    SemaphoreHandle_t lock;
    uint8_t mode_mask;
    uint32_t max_speed_hz;

    int setup(SpiDevice_t *spidev, SpiConf_t *conf);
    int transfer(SpiDevice_t *spidev, SpiMessage_t *msg);
};
typedef struct SpiMaster SpiMaster_t;

struct SpiDevice
{
    Device_t dev;
    SemaphoreHandle_t lock;
    SpiMaster_t *master;
    SpiConf_t conf;
};
typedef struct SpiDevice SpiDevice_t;

struct SpiMessage
{
    struct spi_device *spi;
    uint8_t *tx_buffer;
    uint8_t *rx_buffer;
    int length;
};
typedef struct SpiMessage SpiMessage_t;

#endif /* DRIVERS_SPI_H_ */
    /* vim:set et ts=4 sts=4 sw=4 ft=c: */
