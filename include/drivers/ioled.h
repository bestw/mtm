/**
 * @file led_core.h
 * @brief 
 * @author wj
 * @version 0.1
 * @date 2015-01-19
 */
#ifndef __LED_CORE_H__
#define __LED_CORE_H__

#include <dev_core.h>
#include <dev_os.h>

/** led value */
enum _led_value {
    LED_OFF = 0,
    LED_ON  = 1,
};

/**
 * @brief LED设备数据结构
 */
struct _dev_led {
    struct _dev_struct      dev;        /**> dev */
    enum _led_value         value;
    /** led flag */
#define LED_BLINK_INVERT    (1 << 0)    /** 闪烁一次后常亮 */
#define LED_BLINK_ONCE      (1 << 1)    /** 闪烁一次 */
#define LED_BLINK_ONCE_DONE (1 << 2)    /** 已完成一次闪烁 */
    uint16_t            flag;

    void                *priv_data;
    void (*write)(struct _dev_led *led, enum _led_value value);
    enum _led_value (*read)(struct _dev_led *led);
    const char          *def_trigger_name; /**> default trigger name */

    uint16_t            blink_on_time;  /* ms */
    uint16_t            blink_off_time; /* ms */
    struct _stimer      blink_timer;

    struct _led_trigger *trigger;   /* 当前使用led的trigger */
    struct _list_node   trig_list;  /* 连接到当前的trigger链表上 */
    uint32_t            trig_data;
};

void led_blink_stop(struct _dev_led *led);
void led_blink_set(struct _dev_led *led,
                   uint32_t on_time, uint32_t off_time,
                   BOOL once, BOOL invert);
int led_device_init(struct _dev_led *led);
int led_device_release(struct _dev_led *led);

/**
 * @brief 查找并返回设备名为name的led设备对象指针
 *
 * @param name 设备名
 *
 * @return led设备对象指针, 不存在返回NULL
 */
static __INLINE struct _dev_led *get_led_device(const char *name)
{
    return (struct _dev_led *)NODE_ENTRY(device_find_name(name), struct _dev_led, dev);
}


struct _led_trigger {
    const char          *trigger_name;
    void (*activate)(struct _dev_led *led);
    void (*deactivate)(struct _dev_led *led);

    struct _list_node   node;
    struct _list_node   leds_list; /**< 控制的LED list */
    struct _dev_mutex   leds_list_mutex;
};

void led_trigger_blink(struct _led_trigger *trigger,
                       uint16_t on_time, uint16_t off_time,
                       BOOL once, BOOL invert);
void led_trigger_write(struct _led_trigger *trigger, enum _led_value value);
void led_trigger_set(struct _dev_led *led, struct _led_trigger *trigger);
void led_trigger_set_default(struct _dev_led *led);
int led_trigger_init(struct _led_trigger *trigger);
void led_trigger_release(struct _led_trigger *trigger);

#endif /* __LED_CORE_H__ */
/* END OF FILE */
