/**
 * @file can_core.h
 * @brief 
 * @author WangJun
 * @version 
 * @date 2015-01-20
 */
#ifndef __CAN_CORE_H__
#define __CAN_CORE_H__

#include <dev_core.h>

enum _can_mode {
	CAN_MODE_STOP = 0,
	CAN_MODE_START,
	CAN_MODE_SLEEP
};

enum _can_state {
    CAN_STATE_ERROR_ACTIVE = 0, /**< TEC/REC: 0~95    */
    CAN_STATE_ERROR_WARNING,    /**< TEC/REC: 96~127  */
    CAN_STATE_ERROR_PASSIVE,    /**< TEC/REC: 128~255 */
    CAN_STATE_BUS_OFF,          /**< TEC/REC: > 255   */
    CAN_STATE_STOPPED,
    CAN_STATE_SLEEPING
};

struct _can_bittiming {
    uint32_t bitrate;       /**< bps, bitrate = clock / ((1 + tseg1 + tseg2) * brp) */
    uint16_t sample_point;  /**< sample_point = (1 + tseg1) / (1 + tseg1 + tseg2)*/
    uint32_t tq;            /**< tq = brp * (1 / clock) */
    uint16_t tseg1;         /**< tseg1 = prop_seg + phase_seg1 */
    uint16_t tseg2;         /**< tseg2 = phase_seg2 */
    uint16_t sjw;
    uint16_t brp;           /**< bitrate prescaler */
    uint32_t clock;         /**< CAN system clock frequency in Hz */
};

struct _can_device_stats {
    uint32_t bus_error;
    uint32_t warning;
    uint32_t passive;
    uint32_t bus_off;
    uint32_t arbitration_lost;
    uint32_t restarts;
};

enum _can_err_type {
    CAN_ERR_OK = 0,
    CAN_ERR_Stuff,
    CAN_ERR_Form,
    CAN_ERR_ACK,
    CAN_ERR_BitRecessive,
    CAN_ERR_BitDominant,
    CAN_ERR_CRC,
    CAN_ERR_Unknown,
};

struct _can_err_count {
    uint16_t tec;
    uint16_t rec;
    enum _can_err_type last_err;
};

#define CAN_IDE     (1u << 31) /** 扩展帧 */
#define CAN_RTR     (1u << 30) /** 远程帧 */
struct _can_msg {
    uint32_t can_id;
    uint8_t dlc;
    uint8_t data[8];
};

struct _can_buff {
    struct _list_node   node;
    struct _can_msg     msg;
    uint8_t             life;
};

struct _can_filter {
    uint32_t can_id;
    uint32_t mask; /**< bit=1 是必须匹配位 */
};

struct _dev_can {
    struct _dev_struct      dev;
    struct _can_bittiming   *btm;
    enum _can_mode          mode; /* CAN_MODE_STOP, CAN_MODE_START, CAN_MODE_SLEEP */
    enum _can_state         state;
#define CAN_CTRLMODE_NORMAL         0
#define CAN_CTRLMODE_LOOPBACK       1
#define CAN_CTRLMODE_SILENT         2
    uint16_t                ctrlmode;
    uint16_t                ctrlmode_mask;
    void                    *priv_data;

    struct _can_device_stats    stats;
    struct _can_err_count       err_count;

    struct _list_node       tx_buf_list; /* 硬件还未发送的数据 */
    struct _list_node       rx_buf_list; /* 尚未取走的接收数据 */

    int (*set_bittiming)(struct _dev_can *can, struct _can_bittiming *btm);
    int (*set_mode)(struct _dev_can *can, enum _can_mode mode);
    int (*set_filter)(struct _dev_can *can, struct _can_filter *filter);

    int (*get_state)(struct _dev_can *can);
    int (*get_err_count)(struct _dev_can *can);

    int (*open)(struct _dev_can *can);
    int (*close)(struct _dev_can *can);
    int (*send)(struct _dev_can *can, struct _can_msg *msg);
    int (*hw_tx_delay)(struct _dev_can *can, struct _can_msg *msg);
    int (*recv)(struct _dev_can *can, struct _can_msg *msg);

    int (*hw_tx_timeout)(struct _dev_can *can);

#if CONFIG_CAN_LED > 0
    struct _led_trigger *tx_led_trig;
    struct _led_trigger *rx_led_trig;
#endif
};

void can_calc_bittiming(struct _dev_can *can, struct _can_bittiming *btm);
int can_init(struct _dev_can *can);
int can_release(struct _dev_can *can);

/**
 * @brief 查找并返回设备名为name的can设备对象指针
 *
 * @param name 设备名
 *
 * @return can设备对象指针, 不存在返回NULL
 */
static __INLINE struct _dev_can * get_can_device(const char *name)
{
    return (struct _dev_can *)NODE_ENTRY(device_find_name(name), struct _dev_can, dev);
}

#endif /* __CAN_CORE_H__ */
/* END OF FILE */
