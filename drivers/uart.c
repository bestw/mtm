/**
 * @file uart.c
 * @brief uart device implement.
 * @author WANG Jun
 * @date 2017-06-01
 *
 * history:
 * 2017-06-26   WANG Jun    init version.
 * 2017-07-20   WANG Jun    clean some codes.
 * 2017-08-22   WANG Jun    fixed DMA settings.
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "device.h"

#ifdef INCLUDE_DEVICE_UART

static inline int _UartBufRecv(struct UartDevice *uartdev, uint8_t *data, int length)
{
    int size = 0;
    FW_ASSERT(uartdev != NULL);
    FW_ASSERT(data != NULL);
    FW_ASSERT(uartdev->rx_data != NULL);

    taskENTER_CRITICAL();
    {
        size = RingBufferRead(uartdev->rx_data, data, length, RING_BUFFER_MODE_NONE);
    }
    taskEXIT_CRITICAL();

    return size;
}

static inline int _UartPollRecv(struct UartDevice *uartdev, uint8_t *data, int length)
{
    int size = 0;
    int ch   = -1;
    FW_ASSERT(uartdev != NULL);
    FW_ASSERT(data != NULL);

    size = length;
    while (length > 0)
    {
        ch = uartdev->get_char(uartdev);
        if (ch == -1)
            break;

        *data = ch;
        data++;
        length--;
    }

    return size - length; /* received bytes */
}

static inline int _UartInterruptSend(struct UartDevice *uartdev,
                                     const uint8_t *data, int length)
{
    int size = length;
    FW_ASSERT(uartdev != NULL);
    FW_ASSERT(data != NULL);

    uartdev->tx_data = xTaskGetCurrentTaskHandle();
    while (length > 0)
    {
        if (uartdev->put_char(uartdev, *data) == -1) /* Need Open TC. */
        {
            /* wait tc. */
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            continue;
        }
        data++;
        length--;
    }
    return size - length;
}

static inline int _UartPollSend(struct UartDevice *uartdev,
                                const uint8_t *data, int length)
{
    int size = length;
    FW_ASSERT(uartdev != NULL);
    FW_ASSERT(data != NULL);

    while (length > 0)
    {
        uartdev->put_char(uartdev, *data);
        data++;
        length--;
    }
    return size - length;
}

static inline int _UartDMASend(struct UartDevice *uartdev,
                               const uint8_t *data, int length)
{
    BaseType_t retval   = pdFAIL;
    int send_flag       = 0;
    QueueHandle_t queue = (QueueHandle_t)uartdev->tx_data;
    struct UartQItem item_data;

    FW_ASSERT(uartdev != NULL);
    FW_ASSERT(data != NULL);
    FW_ASSERT(queue != NULL);

    if (xQueuePeek(queue, &item_data, 0) == errQUEUE_EMPTY)
    {
        send_flag = 1;
    }

    item_data.data   = data;
    item_data.length = length;
    retval           = xQueueSend(queue, &item_data, portMAX_DELAY);
    if ((retval == pdPASS) && (send_flag == 1))
    {
        uartdev->control(uartdev, UART_CMD_START_DMA_TX, &item_data);
        return length;
    }

    return 0;
}

int UartISR(struct UartDevice *uartdev, int flag, int param)
{
    switch (flag)
    {
    case UART_ISR_FLAG_RX:
    {
        int ch = -1;
        UBaseType_t sr;
        RingBuffer_t *rx_buf = (RingBuffer_t *)uartdev->rx_data;

        sr = taskENTER_CRITICAL_FROM_ISR();
        while (1)
        {
            ch = uartdev->get_char(uartdev);
            if (ch == -1)
                break;

            RingBufferWrite(rx_buf, &ch, 1, RING_BUFFER_MODE_OVERWRITE);
        }
        taskEXIT_CRITICAL_FROM_ISR(sr);
        break;
    }
    case UART_ISR_FLAG_TX_DONE:
    {
        BaseType_t task_woken = pdFALSE;
        TaskHandle_t tx_task  = (TaskHandle_t)uartdev->tx_data;

        vTaskNotifyGiveFromISR(tx_task, &task_woken);
        portYIELD_FROM_ISR(task_woken);
        break;
    }
    case UART_ISR_FLAG_DMA_TX_DONE:
    {
        struct UartQItem item_data;
        BaseType_t task_worken = pdFALSE;
        QueueHandle_t queue    = (QueueHandle_t)uartdev->tx_data;

        uartdev->control(uartdev, UART_CMD_STOP_DMA_TX, NULL);      /* stop DMA */
        xQueueReceiveFromISR(queue, &item_data, &task_worken);      /* 取出 */
        if (xQueuePeekFromISR(queue, &item_data) != errQUEUE_EMPTY) /* 不取出读取 */
        {
            uartdev->control(uartdev, UART_CMD_START_DMA_TX, &item_data);
        }
        else
        {
            uartdev->control(uartdev, UART_CMD_ENABLE_RS485_RX, NULL); /* enable rs485 rx */
        }
        portYIELD_FROM_ISR(task_worken);
        break;
    }
    case UART_ISR_FLAG_DMA_RX_DONE:
    {
        UBaseType_t sr;
        RingBuffer_t *rb = (RingBuffer_t *)uartdev->rx_data;

        sr = taskENTER_CRITICAL_FROM_ISR();
        {
            RingBufferUpdateWriteIndex(rb, param);
        }
        taskEXIT_CRITICAL_FROM_ISR(sr);
        break;
    }
    default:
        FWLOG_INFO("Flag not support in UART ISR.");
        break;
    }

    return 0;
}

static int UartInit(Device_t *dev)
{
    struct UartDevice *uartdev;

    FW_ASSERT(dev != NULL);
    uartdev = LIST_NODE_ENTRY(dev, struct UartDevice, dev);

#ifdef FW_ENABLE_MALLOC
    uartdev->rx_data = NULL;
    uartdev->tx_data = NULL;
#endif
    if (uartdev->configure != NULL)
    {
        return uartdev->configure(uartdev, &uartdev->cfg);
    }
    return 0;
}

static int UartRelease(Device_t *dev)
{
    struct UartDevice *uartdev;

    FW_ASSERT(dev != NULL);
    uartdev = LIST_NODE_ENTRY(dev, struct UartDevice, dev);

    if (uartdev->configure != NULL)
    {
        return uartdev->configure(uartdev, NULL);
    }
    return 0;
}

static int UartOpen(Device_t *dev, int flags)
{
    struct UartDevice *uartdev;

    FW_ASSERT(dev != NULL);
    uartdev = (struct UartDevice *)dev->data;

    if (uartdev->rx_data == NULL)
    {
        if (flags & DEVICE_FLAG_RX_INT)
        {
#ifdef FW_ENABLE_MALLOC
            uartdev->rx_data = RingBufferCreate(1, uartdev->cfg.buffer_size);
#endif
            uartdev->control(uartdev, UART_CMD_ENABLE_INT_RX, NULL);
            dev->state |= DEVICE_FLAG_RX_INT;
        }
        else if (flags & DEVICE_FLAG_RX_DMA)
        {
#ifdef FW_ENABLE_MALLOC
            uartdev->rx_data = RingBufferCreate(1, uartdev->cfg.buffer_size);
#endif
            uartdev->control(uartdev, UART_CMD_ENABLE_DMA_RX, NULL);
            dev->state |= DEVICE_FLAG_RX_DMA;
        }
        else
        {
            uartdev->rx_data = NULL;
        }
    }

    if (uartdev->tx_data == NULL)
    {
        /* DMA or interrupt */
        if (flags & DEVICE_FLAG_TX_INT)
        {
            uartdev->tx_data = NULL; /* to store TaskHandle */
            dev->state |= DEVICE_FLAG_TX_INT;
        }
        else if (flags & DEVICE_FLAG_TX_DMA)
        {
#ifdef FW_ENABLE_MALLOC
            uartdev->tx_data = xQueueCreate(8, sizeof(struct UartQItem));
#endif
            dev->state |= DEVICE_FLAG_TX_DMA;
        }
        else
        {
            uartdev->tx_data = NULL;
        }
    }
    FWLOG_NOTICE("%s is opened: buadrate %d, databit %d, stopbit %d, parity %d.",
                 dev->name, uartdev->cfg.baudrate, uartdev->cfg.data_bits,
                 uartdev->cfg.stop_bits, uartdev->cfg.parity);

    return 0;
}

static int UartClose(Device_t *dev)
{
    struct UartDevice *uartdev;

    uartdev = (struct UartDevice *)dev->data;

    if (uartdev->rx_data != NULL)
    {
        if (dev->state & DEVICE_FLAG_RX_INT)
        {
#ifdef FW_ENABLE_MALLOC
            RingBufferDestroy((RingBuffer_t *)uartdev->rx_data);
#endif
            uartdev->control(uartdev, UART_CMD_DISABLE_INT_RX, NULL);
            dev->state &= ~DEVICE_FLAG_RX_INT;
        }
        else if (dev->state & DEVICE_FLAG_RX_DMA)
        {
#ifdef FW_ENABLE_MALLOC
            RingBufferDestroy((RingBuffer_t *)uartdev->rx_data);
#endif
            uartdev->control(uartdev, UART_CMD_DISABLE_DMA_RX, NULL);
            dev->state &= ~DEVICE_FLAG_RX_DMA;
        }
        uartdev->rx_data = NULL;
    }

    if (uartdev->tx_data != NULL)
    {
        /* DMA or interrupt */
        if (dev->state & DEVICE_FLAG_TX_INT)
        {
            dev->state &= ~DEVICE_FLAG_TX_INT;
        }
        else if (dev->state & DEVICE_FLAG_TX_DMA)
        {
            vQueueDelete((QueueHandle_t)uartdev->tx_data);
            dev->state &= ~DEVICE_FLAG_TX_DMA;
        }
        uartdev->tx_data = NULL;
    }

    FWLOG_NOTICE("%s is closed.", dev->name);

    return 0;
}

static int UartRead(Device_t *dev, void *buf, int length)
{
    struct UartDevice *uartdev = LIST_NODE_ENTRY(dev, struct UartDevice, dev);
    int count;

    if (dev->state & DEVICE_FLAG_RX_INT)
    {
        count = _UartBufRecv(uartdev, (uint8_t *)buf, length);
    }
    else if (dev->state & DEVICE_FLAG_RX_DMA)
    {
        count = _UartBufRecv(uartdev, (uint8_t *)buf, length);
    }
    else
    {
        count = _UartPollRecv(uartdev, (uint8_t *)buf, length);
    }
    return count;
}

static int UartWrite(Device_t *dev, const void *buf, int length)
{
    struct UartDevice *uartdev = LIST_NODE_ENTRY(dev, struct UartDevice, dev);
    int count;

    uartdev->control(uartdev, UART_CMD_DISABLE_RS485_RX, NULL);
    if (dev->state & DEVICE_FLAG_TX_INT)
    {
        count = _UartInterruptSend(uartdev, (const uint8_t *)buf, length);
        uartdev->control(uartdev, UART_CMD_ENABLE_RS485_RX, NULL);
    }
    else if (dev->state & DEVICE_FLAG_TX_DMA)
    {
        count = _UartDMASend(uartdev, (const uint8_t *)buf, length);
    }
    else
    {
        count = _UartPollSend(uartdev, (const uint8_t *)buf, length);
        uartdev->control(uartdev, UART_CMD_ENABLE_RS485_RX, NULL);
    }

    return count;
}

static int UartIoctl(Device_t *dev, int cmd, void *args)
{
    struct UartDevice *uartdev;
    uartdev = (struct UartDevice *)dev->data;
    switch (cmd)
    {
    case UART_CMD_CONFIG:
        uartdev->configure(uartdev, (struct UartConfig *)args);
        break;
    default:
        if (uartdev->control != NULL)
            uartdev->control(uartdev, cmd, args);
        break;
    }
    return 0;
}

int UartInstall(struct UartDevice *uartdev, const char *name, int flags)
{
    Device_t *dev = &uartdev->dev;

    dev->init    = UartInit;
    dev->release = UartRelease;
    dev->open    = UartOpen;
    dev->close   = UartClose;
    dev->read    = UartRead;
    dev->write   = UartWrite;
    dev->ioctl   = UartIoctl;

    dev->data = uartdev;
    return DeviceInstall(dev, name, flags);
}

#endif /* INCLUDE_DEVICE_UART */
/* vim:set et ts=4 sts=4 sw=4 ft=c: */
