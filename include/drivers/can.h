/**
 * @file
 * @brief
 * @author
 * @version v1.0
 * @date 2017-09-07
 */
#ifndef DRIVERS_CAN_H_
#define DRIVERS_CAN_H_

#include "frameworks.h"

#define CAN_CMD_CONFIG 0x01
#define CAN_CMD_SET_INT 0x02
#define CAN_CMD_CRL_INT 0x03
#define CAN_CMD_SET_FILTER 0x04
#define CAN_CMD_SET_MODE 0x05
#define CAN_CMD_SET_BAUD 0x06
#define CAN_CMD_SET_SAMPLEPOINT 0x07

#define CAN_CTRLMODE_NORMAL 0x00          /* Normal mode */
#define CAN_CTRLMODE_LOOPBACK 0x01        /* Loopback mode */
#define CAN_CTRLMODE_SILENT 0x02          /* Silent(Listen-only) mode */
#define CAN_CTRLMODE_SILENT_LOOPBACK 0x03 /* Loopback combined with silent mode */
#define CAN_CTRLMODE_3_SAMPLES 0x04       /* Triple sampling mode */
#define CAN_CTRLMODE_ONE_SHOT 0x05        /* One-Shot mode */

/*
 * CAN mode
 */
enum CanMode
{
    CAN_MODE_STOP = 0,
    CAN_MODE_START,
    CAN_MODE_SLEEP
};

enum CanBaud
{
    kCanBaudRate_1M   = 1000UL * 1000, /* 1 MBit/sec   */
    kCanBaudRate_800k = 1000UL * 800,  /* 800 kBit/sec */
    kCanBaudRate_500k = 1000UL * 500,  /* 500 kBit/sec */
    kCanBaudRate_250k = 1000UL * 250,  /* 250 kBit/sec */
    kCanBaudRate_125k = 1000UL * 125,  /* 125 kBit/sec */
    kCanBaudRate_100k = 1000UL * 100,  /* 100 kBit/sec */
    kCanBaudRate_50k  = 1000UL * 50,   /* 50 kBit/sec  */
    kCanBaudRate_20k  = 1000UL * 20,   /* 20 kBit/sec  */
    kCanBaudRate_10k  = 1000UL * 10    /* 10 kBit/sec  */
};
/*
 * CAN operational and error states
 */
enum CanState
{
    CAN_STATE_ERROR_ACTIVE = 0, /* RX/TX error count < 96 */
    CAN_STATE_ERROR_WARNING,    /* RX/TX error count < 128 */
    CAN_STATE_ERROR_PASSIVE,    /* RX/TX error count < 256 */
    CAN_STATE_BUS_OFF,          /* RX/TX error count >= 256 */
    CAN_STATE_STOPPED,          /* Device is stopped */
    CAN_STATE_SLEEPING,         /* Device is sleeping */
    CAN_STATE_MAX
};

typedef struct CanMessage CanMsg_t;
struct CanMessage
{
    uint32_t can_id;
    uint8_t ide;
    uint8_t rtr;
    uint8_t len;
    uint8_t res0;
    uint8_t data[8];
};

struct CanClock
{
    uint32_t freq; /* system clock, unit:Hz */
};
/*
 * CAN harware-dependent bit-timing constant
 *
 * Used for calculating and checking bit-timing parameters
 */
struct CanBittimingConst
{
    uint32_t tseg1_min; /* Time segement 1 = prop_seg + phase_seg1 min value */
    uint32_t tseg1_max; /* max value */
    uint32_t tseg2_min; /* Time segement 2 = phase_seg2 min value */
    uint32_t tseg2_max; /* max value */
    uint32_t sjw_max;   /* Synchronisation jump width max value*/
    uint32_t brp_min;   /* Bit-rate prescaler min value */
    uint32_t brp_max;   /* Bit-rate prescaler max value */
    uint32_t brp_inc;   /* Bit-rate prescaler, increment */
};

/* stm32:
 * Prescaler = brp;
 * BS1 = prop_seg + phase_seg1;
 * BS2 = phase_seg2;
 * SJW = sjw;
 * */
struct CanBittiming
{
    uint32_t bitrate;      /* unit: bps */
    uint32_t brp;          /* Bit-rate prescaler, stm32:[0:9],sja1000:[0:3] */
    uint32_t tq;           /* Time quanta (TQ) in nanoseconds */
    uint32_t prop_seg;     /* Propagation segment in TQs */
    uint32_t phase_seg1;   /* Phase buffer segment 1 in TQs */
    uint32_t phase_seg2;   /* Phase buffer segment 2 in TQs */
    uint32_t sjw;          /* Synchronisation jump width in TQs, 1~4 */
    uint32_t sample_point; /* Sample point, uint: one-tenth of a percent */
};

struct CanStatus
{
    uint32_t bus_sts;
    uint32_t error_sts;
    uint32_t send_sts;
    uint32_t recv_sts;
};

/*
 * CAN filters
 */
struct CanFilterItem
{
    uint32_t can_id : 29;
    uint32_t ide : 1; /* ide = 0: standard frame; ide = 1: extended frame */
    uint32_t rtr : 1; /* rtr = 0: data frame; rtr = 1: remote frame */
    uint32_t res : 1;
    uint32_t mask;
};

struct CanFilterConfig
{
    uint32_t count;                   /* count */
    struct CanFilterItem filter_item; /* can_id and mask */
};

struct CanDevice
{
    Device_t dev;
    struct CanClock clock;
    struct CanBittiming can_bt;
    struct CanBittimingConst can_btc;
    struct CanFilterConfig can_filter;

    uint32_t can_mode;

    void *can_rx;
    void *can_tx;

    int (*configure)(struct CanDevice *candev);
    int (*control)(struct CanDevice *candev, int cmd, void *args);
    int (*sendmessage)(struct CanDevice *candev, CanMsg_t *can_msg); /* 须保证此指针指向的函数可在中断中运行 */
};

int CanInstall(struct CanDevice *candev, const char *name, int flags);
void CanReceiveISR(struct CanDevice *candev, struct CanMessage *message);
#endif /* DRIVERS_CAN_H_ */
/* END OF FILE */
