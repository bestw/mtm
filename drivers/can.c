
#include "frameworks.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "device.h"

#ifdef INCLUDE_DEVICE_CAN

#define FW_CAN1_RXBUF_SIZE 8
#define FW_CAN1_TXBUF_SIZE 8

//@TODO: 未做直接配置bittiming，这里只选择计算方式
#ifndef CONFIG_CAN_CALC_BITTIMING
#define CONFIG_CAN_CALC_BITTIMING
#endif

#ifdef CONFIG_CAN_CALC_BITTIMING
#define CAN_CALC_MAX_ERROR 50 /* in one-tenth of a percent */

/*
 * Bit-timing calculation derived from:
 * Calculates proper bit-timing parameters for a specified bit-rate
 * and sample-point, which can then be used to set the bit-timing
 * registers of the CAN controller. You can find more information
 * in the header frameworks/include/drivers/can.h.
 * @input: CanBittimingConst, sampl_pt, tseg
 * @output: tseg1, tseg2, 
 * @return: sample_point;
 */
static inline int _CanUpdateSamplePoint(const struct CanBittimingConst *btc,
                                        int sampl_pt, int tseg, int *tseg1, int *tseg2)
{
    /*
     * tseg = *tseg1 + *tseg2
     *
     *            *tseg1 + 1
     * sampl_pt = ----------- * 1000
     *             tseg + 1
    */
    *tseg2 = tseg + 1 - (sampl_pt * (tseg + 1)) / 1000;
    if (*tseg2 < btc->tseg2_min)
        *tseg2 = btc->tseg2_min;
    if (*tseg2 > btc->tseg2_max)
        *tseg2 = btc->tseg2_max;
    *tseg1 = tseg - *tseg2;
    if (*tseg1 > btc->tseg1_max)
    {
        *tseg1 = btc->tseg1_max;
        *tseg2 = tseg - *tseg1;
    }
    return 1000 * (tseg + 1 - *tseg2) / (tseg + 1);
}
/*
 * Calculates proper bit-timing parameters for a specified bit-rate
 * and sample-point, which can then be used to set the bit-timing
 * registers of the CAN controller. You can find more information
 * in the header file frameworks/include/drivers/can.h.
 * Mask do_div();
 */
static inline int _CanCalcBittiming(struct CanDevice *candev, struct CanBittiming *bt,
                                    const struct CanBittimingConst *btc)
{
    long best_error = 1000000000, error = 0;
    int best_tseg = 0, best_brp = 0, brp = 0;
    int tsegall, tseg = 0, tseg1 = 0, tseg2 = 0;
    int spt_error = 1000, spt = 0, sampl_pt;
    long rate;
    // uint64_t v64;

    /* Use CiA recommended sample points */
    if (bt->sample_point)
    {
        sampl_pt = bt->sample_point;
    }
    else
    {
        if (bt->bitrate > 800000)
            sampl_pt = 750;
        else if (bt->bitrate > 500000)
            sampl_pt = 800;
        else
            sampl_pt = 875;
    }

    /* tseg even = round down, odd = round up */
    for (tseg = (btc->tseg1_max + btc->tseg2_max) * 2 + 1;
         tseg >= (btc->tseg1_min + btc->tseg2_min) * 2; tseg--)
    {
        tsegall = 1 + tseg / 2;
        /* Compute all possible tseg choices (tseg=tseg1+tseg2) */
        brp = candev->clock.freq / (tsegall * bt->bitrate) + tseg % 2;
        /* chose brp step which is possible in system */
        brp = (brp / btc->brp_inc) * btc->brp_inc;
        if ((brp < btc->brp_min) || (brp > btc->brp_max))
            continue;
        rate  = candev->clock.freq / (brp * tsegall);
        error = bt->bitrate - rate;
        /* tseg brp biterror */
        if (error < 0)
            error = -error;
        if (error > best_error)
            continue;
        best_error = error;
        if (error == 0)
        {
            spt   = _CanUpdateSamplePoint(btc, sampl_pt, tseg / 2,
                                        &tseg1, &tseg2);
            error = sampl_pt - spt;
            if (error < 0)
                error = -error;
            if (error > spt_error)
                continue;
            spt_error = error;
        }
        best_tseg = tseg / 2;
        best_brp  = brp;
        if (error == 0)
            break;
    }

    if (best_error)
    {
        /* Error in one-tenth of a percent */
        error = (best_error * 1000) / bt->bitrate;
        if (error > CAN_CALC_MAX_ERROR)
        {
            FWLOG_ERR("bitrate error %3d.%1d%% too high\n",
                      error / 10, error % 10);
            return -E_ERROR; /* EDOM */
        }
        else
        {
            FWLOG_WARNING("bitrate error %3d.%1d%%\n",
                          error / 10, error % 10);
        }
    }

    /* real sample point */
    bt->sample_point = _CanUpdateSamplePoint(btc, sampl_pt, best_tseg,
                                             &tseg1, &tseg2);
    //@TODO:未做64位除法，不使用tq作为输入进行bittiming计算
    //    v64 = (uint64_t)best_brp * 1000000000UL; /* for get int (in nanoseconds) */
    //    do_div(v64, (uint64_t)candev->clock.freq);
    //    bt->tq         = (uint32_t)v64;
    bt->prop_seg   = tseg1 / 2;
    bt->phase_seg1 = tseg1 - bt->prop_seg;
    bt->phase_seg2 = tseg2;

    /* check for sjw user settings */
    if (!bt->sjw || !btc->sjw_max)
        bt->sjw = 1;
    else
    {
        /* bt->sjw is at least 1 -> sanitize upper bound to sjw_max */
        if (bt->sjw > btc->sjw_max)
            bt->sjw = btc->sjw_max;
        /* bt->sjw must not be higher than tseg2 */
        if (tseg2 < bt->sjw)
            bt->sjw = tseg2;
    }

    bt->brp = best_brp;
    /* real bit-rate */
    bt->bitrate = candev->clock.freq / (bt->brp * (tseg1 + tseg2 + 1));

    return 0;
}
#else  /* !CONFIG_CAN_CALC_BITTIMING */
static inline int _CanCalcBittiming(struct CanDevice *candev, struct CanBittiming *bt,
                                    const struct CanBittimingConst *btc)
{
    FWLOG_ERR("bit-timing calculation not available\n");
    return -E_ERROR; /* invalid */
}
#endif /* CONFIG_CAN_CALC_BITTIMING */
/*
 * Checks the validity of the specified bit-timing parameters prop_seg,
 * phase_seg1, phase_seg2 and sjw and tries to determine the bitrate
 * prescaler value brp. You can find more information in the header
 * file frameworks/include/drivers/can.h.
 * Mask do_div();
 */
static inline int _CanFixupBittiming(struct CanDevice *candev, struct CanBittiming *bt,
                                     const struct CanBittimingConst *btc)
{
    int tseg1, alltseg;
    uint64_t brp64;

    tseg1 = bt->prop_seg + bt->phase_seg1;
    if (!bt->sjw)
        bt->sjw = 1;
    if (bt->sjw > btc->sjw_max ||
        tseg1 < btc->tseg1_min || tseg1 > btc->tseg1_max ||
        bt->phase_seg2 < btc->tseg2_min || bt->phase_seg2 > btc->tseg2_max)
        return -E_ERROR; /* out of the range */

    brp64 = (uint64_t)candev->clock.freq * (uint64_t)bt->tq;
    if (btc->brp_inc > 1)
    {
        /* do_div(brp64, (uint64_t)btc->brp_inc); */
    }

    brp64 += 500000000UL - 1;
    //@TODO:未做64位除法，不使用tq作为输入进行bittiming计算
    //    do_div(brp64, 1000000000UL); /* the practicable BRP */
    if (btc->brp_inc > 1)
        brp64 *= btc->brp_inc;
    bt->brp = (uint32_t)brp64;
    brp64 += 500000000UL - 1;
    //    do_div(brp64, 1000000000UL); /* the practicable BRP */
    if (btc->brp_inc > 1)
        brp64 *= btc->brp_inc;
    bt->brp = (uint32_t)brp64;

    if (bt->brp < btc->brp_min || bt->brp > btc->brp_max)
        return -E_ERROR; /* invalid */

    alltseg          = bt->prop_seg + bt->phase_seg1 + bt->phase_seg2 + 1;
    bt->bitrate      = candev->clock.freq / (bt->brp * alltseg);
    bt->sample_point = ((tseg1 + 1) * 1000) / alltseg;

    return 0;
}
/*
 * @input: candev->clock
 * @input: bt->bitrate ( bt->tq = 0 )
 * @input: btc
 * @output: bt ( but bt->bitrate and bt->tq )
 * Mask using CanFixupBittiming();
 */
static int CanGetBittiming(struct CanDevice *candev, struct CanBittiming *bt,
                           const struct CanBittimingConst *btc)
{
    int err;

    /* Check if the CAN device has bit-timing parameters */
    if (!btc)
        return -E_EMPTY; /* EOPNOTSUPP */

    /*
    * Depending on the given can_bittiming parameter structure the CAN
    * timing parameters are calculated based on the provided bitrate OR
    * alternatively the CAN timing parameters (tq, prop_seg, etc.) are
    * provided directly which are then checked and fixed up.
    */
    //@TODO:未做64位除法，不使用tq作为输入进行bittiming计算
    if (!bt->tq && bt->bitrate)
        err = _CanCalcBittiming(candev, bt, btc);
    else if (bt->tq && !bt->bitrate)
        err = -E_EMPTY;
    //@TODO:不使用tq作为输入进行bittiming计算
    //err = _CanFixupBittiming(candev, bt, btc);
    else
        err = -E_ERROR; /* invalid */

    return err;
}

static int CanIoctl(Device_t *dev, int cmd, void *args)
{
    struct CanDevice *candev;
    struct CanBittimingConst btc_tmp = {
        0,
    };
    struct CanBittiming *bt_tmp;
    struct CanFilterConfig *pfilter;
    uint32_t argval;

    FW_ASSERT(dev != NULL);
    candev = LIST_NODE_ENTRY(dev, struct CanDevice, dev);

    switch (cmd)
    {
    case CAN_CMD_CONFIG:
        bt_tmp  = args;
        btc_tmp = candev->can_btc;
        CanGetBittiming(candev, bt_tmp, &btc_tmp);
        candev->can_bt = *bt_tmp;

        candev->configure(candev);
        break;
    case CAN_CMD_SET_INT:
        argval = (uint32_t)args;
        if (argval &= DEVICE_FLAG_RX_INT)
        {
            candev->control(candev, CAN_CMD_SET_INT, (void *)DEVICE_FLAG_RX_INT);
        }
        if (argval &= DEVICE_FLAG_TX_INT)
        {
            candev->control(candev, CAN_CMD_SET_INT, (void *)DEVICE_FLAG_TX_INT);
        }
        break;
    case CAN_CMD_CRL_INT:
        argval = (uint32_t)args;
        if (argval &= DEVICE_FLAG_RX_INT)
        {
            candev->control(candev, CAN_CMD_CRL_INT, (void *)DEVICE_FLAG_RX_INT);
        }
        if (argval &= DEVICE_FLAG_TX_INT)
        {
            candev->control(candev, CAN_CMD_CRL_INT, (void *)DEVICE_FLAG_TX_INT);
        }
        break;
    //TODO:set fitlter命令待完善
    case CAN_CMD_SET_FILTER:
        pfilter = (struct CanFilterConfig *)args;
        if (candev->can_filter.filter_item.ide == 0)
        {
            pfilter->filter_item.ide = 0;
        }
        else
        {
            pfilter->filter_item.ide = 1;
        }
        if (candev->can_filter.filter_item.rtr == 0)
        {
            pfilter->filter_item.rtr = 0;
        }
        else
        {
            pfilter->filter_item.rtr = 1;
        }

        candev->control(candev, CAN_CMD_SET_FILTER, pfilter);
        break;
    case CAN_CMD_SET_MODE:
        candev->control(candev, CAN_CMD_SET_MODE, args);
        break;
    case CAN_CMD_SET_BAUD:
        argval = (uint32_t)args;
        if ((argval != kCanBaudRate_1M) &&
            (argval != kCanBaudRate_800k) &&
            (argval != kCanBaudRate_500k) &&
            (argval != kCanBaudRate_250k) &&
            (argval != kCanBaudRate_125k) &&
            (argval != kCanBaudRate_100k) &&
            (argval != kCanBaudRate_50k) &&
            (argval != kCanBaudRate_20k) &&
            (argval != kCanBaudRate_10k))
        {
            return -1;
        }
        btc_tmp                = candev->can_btc;
        candev->can_bt.bitrate = argval;
        CanGetBittiming(candev, &candev->can_bt, &btc_tmp);
        candev->control(candev, CAN_CMD_SET_BAUD, args);
        break;
    //TODO:待优化
    case CAN_CMD_SET_SAMPLEPOINT:
        argval = (uint32_t)args;
        if (argval > 1000)
        {
            return -1;
        }
        btc_tmp                     = candev->can_btc;
        candev->can_bt.sample_point = argval;
        CanGetBittiming(candev, &candev->can_bt, &btc_tmp);
        candev->control(candev, CAN_CMD_SET_SAMPLEPOINT, args);
        break;
    default:
        break;
    }
    return 0;
}

static int CanInit(Device_t *dev)
{
    struct CanDevice *candev;

    int err = 0;

    FW_ASSERT(dev != NULL);
    candev = LIST_NODE_ENTRY(dev, struct CanDevice, dev);

#ifdef FW_ENABLE_MALLOC
    candev->can_rx = NULL;
    candev->can_tx = NULL;
#endif

    if (candev->configure != NULL)
    {
        CanIoctl(dev, CAN_CMD_CONFIG, &(candev->can_bt));
    }
    else
    {
        FWLOG_ERR("bit-timing constants is not available\n");
        err = -E_EMPTY;
    }

    return err;
}

static int CanOpen(Device_t *dev, int flags)
{
    struct CanDevice *candev;

    FW_ASSERT(dev != NULL);
    candev = LIST_NODE_ENTRY(dev, struct CanDevice, dev);

    if (flags & DEVICE_FLAG_RX_INT)
    {
        if (candev->can_rx == NULL)
        {
            candev->can_rx = RingBufferCreate(sizeof(struct CanMessage), FW_CAN1_RXBUF_SIZE);
        }
        candev->control(candev, CAN_CMD_SET_INT, (void *)DEVICE_FLAG_RX_INT);
    }

    if (flags & DEVICE_FLAG_TX_INT)
    {
        if (candev->can_tx == NULL)
        {
            candev->can_tx = RingBufferCreate(sizeof(struct CanMessage), FW_CAN1_TXBUF_SIZE);
        }
        candev->control(candev, CAN_CMD_SET_INT, (void *)DEVICE_FLAG_TX_INT);
    }
    FWLOG_NOTICE("%s is opened: buadrate %dkbps, samplepoint %.1f%%",
                 dev->name, candev->can_bt.bitrate / 1000, ((float)candev->can_bt.sample_point / 10));

    return 0;
}
static int CanClose(Device_t *dev)
{
    struct CanDevice *candev;
    int flags;

    FW_ASSERT(dev != NULL);
    candev = LIST_NODE_ENTRY(dev, struct CanDevice, dev);

    flags = dev->flags;
    if (flags & DEVICE_FLAG_RX_INT)
    {
#ifdef FW_ENABLE_MALLOC
        if (candev->can_rx != NULL)
        {
            RingBufferDestroy((RingBuffer_t *)candev->can_rx);
        }
#endif

        candev->control(candev, CAN_CMD_CRL_INT, (void *)DEVICE_FLAG_RX_INT);
        dev->state &= ~DEVICE_FLAG_RX_INT;
    }

    if (flags & DEVICE_FLAG_TX_INT)
    {
#ifdef FW_ENABLE_MALLOC
        if (candev->can_tx != NULL)
        {
            RingBufferDestroy((RingBuffer_t *)candev->can_tx);
        }
#endif

        candev->control(candev, CAN_CMD_CRL_INT, (void *)DEVICE_FLAG_TX_INT);
        dev->state &= ~DEVICE_FLAG_TX_INT;
    }

    return 0;
}

static int CanSend(Device_t *dev, const void *buf, int size)
{
    struct CanDevice *candev;
    int count = 0;

    FW_ASSERT(buf != NULL);
    FW_ASSERT(dev != NULL);
    candev = LIST_NODE_ENTRY(dev, struct CanDevice, dev);

    if (candev->sendmessage(candev, (struct CanMessage *)buf) != 0)
    {
        FWLOG_ERR("%s send message failed.", dev->name);
    }
    else
    {
        count = 1;
    }

    return count;
}

static int CanReceive(Device_t *dev, void *buf, int size)
{
    struct CanDevice *candev;
    int count;

    FW_ASSERT(dev != NULL);
    candev = LIST_NODE_ENTRY(dev, struct CanDevice, dev);

    taskENTER_CRITICAL();
    count = RingBufferRead((RingBuffer_t *)candev->can_rx, buf, size, RING_BUFFER_MODE_NONE);
    taskEXIT_CRITICAL();

    return count;
}

void CanReceiveISR(struct CanDevice *candev, struct CanMessage *message)
{
    int count;
    UBaseType_t sr;

    FW_ASSERT(candev != NULL);
    FW_ASSERT(candev->can_rx != NULL);
    FW_ASSERT(message != NULL);

    sr    = taskENTER_CRITICAL_FROM_ISR();
    count = RingBufferQueryFree((RingBuffer_t *)candev->can_rx);
    RingBufferWrite((RingBuffer_t *)candev->can_rx, message, 1, RING_BUFFER_MODE_OVERWRITE);
    taskEXIT_CRITICAL_FROM_ISR(sr);
    if (count == 0)
    {
        FWLOG_WARNING("CAN RX buffer is full.");
    }
}

int CanInstall(struct CanDevice *candev, const char *name, int flags)
{
    Device_t *dev = &candev->dev;

    dev->init    = CanInit;
    dev->release = NULL;
    dev->open    = CanOpen;
    dev->close   = CanClose;
    dev->read    = CanReceive;
    dev->write   = CanSend;
    dev->ioctl   = CanIoctl;

    dev->data = candev;
    return DeviceInstall(dev, name, flags);
}

#endif /* INCLUDE_DEVICE_CAN */
