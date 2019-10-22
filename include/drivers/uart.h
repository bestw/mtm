/**
 * @file uart.h
 * @brief 
 * @author WANG Jun
 * @version v1.0
 * @date 2017-05-31
 *
 * update:
 * 2017-07-20   WANG Jun    delete default config.
 */
#ifndef FW_DRIVERS_UART_H_
#define FW_DRIVERS_UART_H_

#include "frameworks.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Uart config defines */
#define UART_CFG_BAUDRATE_2400 2400
#define UART_CFG_BAUDRATE_9600 9600
#define UART_CFG_BAUDRATE_19200 19200
#define UART_CFG_BAUDRATE_38400 38400
#define UART_CFG_BAUDRATE_57600 57600
#define UART_CFG_BAUDRATE_115200 115200
#define UART_CFG_BAUDRATE_230400 230400
#define UART_CFG_BAUDRATE_460800 460800
#define UART_CFG_BAUDRATE_921600 921600
#define UART_CFG_BAUDRATE_2250000 2250000

#define UART_CFG_DATABITS_8 8
#define UART_CFG_DATABITS_9 9

#define UART_CFG_STOPBITS_1 1
#define UART_CFG_STOPBITS_2 2

#define UART_CFG_PARITY_NO 0
#define UART_CFG_PARITY_ODD 1
#define UART_CFG_PARITY_EVEN 2

#define UART_CFG_FLOWCONTROL_NONE 0
#define UART_CFG_FLOWCONTROL_RTS 1
#define UART_CFG_FLOWCONTROL_CTS 2
#define UART_CFG_FLOWCONTROL_RTS_CTS 3

struct UartConfig
{
    int baudrate;
    uint8_t data_bits;
    uint8_t stop_bits;
    uint8_t parity;
    uint8_t flow_control;
    int buffer_size;
};

struct UartQItem
{
    const uint8_t *data;
    int length;
};

// clang-format off
/* Uart device cmds */
#define UART_CMD_CONFIG             1
#define UART_CMD_START_DMA_TX       2
#define UART_CMD_STOP_DMA_TX        3
#define UART_CMD_ENABLE_DMA_RX      4
#define UART_CMD_DISABLE_DMA_RX     5
#define UART_CMD_ENABLE_INT_RX      6
#define UART_CMD_DISABLE_INT_RX     7
#define UART_CMD_ENABLE_RS485_RX    8
#define UART_CMD_DISABLE_RS485_RX   9

/* Uart isr flags */
#define UART_ISR_FLAG_RX            1
#define UART_ISR_FLAG_TX_DONE       2
#define UART_ISR_FLAG_DMA_TX_DONE   3
#define UART_ISR_FLAG_DMA_RX_DONE   4
// clang-format on

struct UartDevice
{
    Device_t dev;
    struct UartConfig cfg;
    void *rx_data;
    void *tx_data;

    /* return -1 if none */
    int (*get_char)(struct UartDevice *uartdev);
    /* return -1 if busy, 0 if successed */
    int (*put_char)(struct UartDevice *uartdev, uint8_t ch);
    int (*configure)(struct UartDevice *uartdev, struct UartConfig *cfg);
    int (*control)(struct UartDevice *uartdev, int cmd, void *args);
};

int UartISR(struct UartDevice *uartdev, int flags, int param);
int UartInstall(struct UartDevice *uartdev, const char *name, int flags);

#ifdef __cplusplus
}
#endif

#endif /* FW_DRIVERS_UART_H_ */
/* vim:set et ts=4 sts=4 sw=4 ft=c: */
