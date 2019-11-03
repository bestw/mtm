/**
 * @file device.c
 * @brief basic device management functions.
 * @author WANG Jun
 * @version v1.0
 * @date 2017-06-01
 *
 * update
 * 2107-07-19   WANG Jun    update defines.
 */

#include "FreeRTOS.h"
#include "task.h"
#include "device.h"

#ifdef INCLUDE_DEVICE

/**
 * @brief 设备链接节点，所有安装到系统的驱动设备都挂在这里。
 */
static struct list_head sDevicesList = LIST_INITIALIZER(sDevicesList);

Device_t *DeviceFindByName(const char *name)
{
    Device_t *dev = NULL;
    struct list_head *plist = NULL;
    FW_ASSERT(name != NULL);

    taskENTER_CRITICAL();
    {
        if (!ListIsEmpty(&sDevicesList))
        {
            LIST_FOR_EACH(plist, &sDevicesList)
            {
                dev = LIST_NODE_ENTRY(plist, Device_t, node);
                if (strncmp(name, dev->name, FW_MAX_NAME_LENGTH) == 0)
                {
                    taskEXIT_CRITICAL();
                    return dev;
                }
            }
        }
    }
    taskEXIT_CRITICAL();

    return NULL;
}

void DeviceList(int argc, char **argv)
{
    printk(" Address     DeviceName    State\n");
    printk("==========  =============  ======\n");
    taskENTER_CRITICAL();
    {
        if (!ListIsEmpty(&sDevicesList))
        {
            Device_t *dev          = NULL;
            struct list_head *each = NULL;
            LIST_FOR_EACH(each, &sDevicesList)
            {
                dev = LIST_NODE_ENTRY(each, Device_t, node);
                printk("%-10X   %-12s   %4X\n", (unsigned int)dev, dev->name, dev->state);
            }
        }
    }
    taskEXIT_CRITICAL();
    printk("\n");
}

int DeviceInstall(Device_t *dev, const char *name, int flags)
{
    FW_ASSERT(dev != NULL);
    FW_ASSERT(name != NULL);

    if (DeviceFindByName(name) != NULL)
    {
        FWLOG_ERR("Install device <%s> error, name existed.", name);
        return -1;
    }

    dev->name        = name;
    dev->flags       = flags & 0xFFFF;
    dev->state       = 0;
    dev->check_value = DEVICE_CHECK_VALUE;

    taskENTER_CRITICAL();
    {
        ListAddHead(&dev->node, &sDevicesList);
    }
    taskEXIT_CRITICAL();

    return 0;
}

int DeviceRemove(Device_t *dev)
{
    FW_ASSERT(dev != NULL);

    if (dev->check_value != DEVICE_CHECK_VALUE)
    {
        FWLOG_ERR("Check value error: 0x%4X.", dev->check_value);
        return -1;
    }

    if (DeviceFindByName(dev->name) != dev)
    {
        FWLOG_ERR("Device <%s> is NOT installed.", dev->name);
        return -1;
    }

    if (dev->state & DEVICE_STATE_INITIALIZED)
    {
        FWLOG_WARNING("Device <%s> is IN USE, will be released...", dev->name);
        DeviceRelease(dev);
    }
    taskENTER_CRITICAL();
    {
        ListDelete(&dev->node);
    }
    taskEXIT_CRITICAL();

    return 0;
}

int DeviceGetState(Device_t *dev)
{
    FW_ASSERT(dev != NULL);
    return (int)dev->state;
}

int DeviceInit(Device_t *dev)
{
    int retval = 0;
    FW_ASSERT(dev != NULL);

    if (dev->check_value != DEVICE_CHECK_VALUE)
    {
        FWLOG_ERR("Check value error: 0x%4X.", dev->check_value);
        return -1;
    }

    if (dev->state & DEVICE_STATE_INITIALIZED)
    {
        return retval;
    }

    if (dev->init == NULL)
    {
        /* ALL DEVICES must have init function! */
        FWLOG_ERR("Initialize device <%s> failed: NO init function.", dev->name);
        return -1;
    }
    retval = dev->init(dev);

    if (retval != 0)
    {
        FWLOG_ERR("Init device <%s> failed, return: %d.", dev->name, retval);
    }
    else
    {
        dev->state = DEVICE_STATE_INITIALIZED;
    }

    return retval;
}

int DeviceRelease(Device_t *dev)
{
    int retval = 0;
    FW_ASSERT(dev != NULL);

    if (dev->check_value != DEVICE_CHECK_VALUE)
    {
        FWLOG_ERR("Check value error: 0x%4X.", dev->check_value);
        return -1;
    }

    if (!(dev->state & DEVICE_STATE_INITIALIZED))
    {
        return retval;
    }

    if (dev->state & DEVICE_STATE_OPENED)
    {
        DeviceClose(dev);
    }

    if (dev->release != NULL)
    {
        retval = dev->release(dev);
        if (retval != 0)
        {
            FWLOG_ERR("Device release error: %d.", retval);
        }
    }
    dev->state = 0;
    return retval;
}

int DeviceOpen(Device_t *dev, int flags)
{
    int retval = 0;
    FW_ASSERT(dev != NULL);

    if (dev->check_value != DEVICE_CHECK_VALUE)
    {
        FWLOG_ERR("Check value error: 0x%4X.", dev->check_value);
        return -1;
    }

    if ((flags & dev->flags) != flags)
    {
        FWLOG_ERR("Cannot open device, exist non-supported flags.");
        return -1;
    }

    if (!(dev->state & DEVICE_STATE_INITIALIZED))
    {
        DeviceInit(dev);
    }

    if (dev->state & DEVICE_STATE_OPENED)
    {
        return -1;
    }

    dev->state &= ~DEVICE_FLAG_MASK;
    if (dev->open != NULL)
    {
        retval = dev->open(dev, flags);
    }
    if (retval != 0)
    {
        FWLOG_ERR("Device <%s> opened error, return: %d.", retval);
        return retval;
    }
    dev->state |= DEVICE_STATE_OPENED;
    return retval;
}

int DeviceClose(Device_t *dev)
{
    int retval = 0;
    FW_ASSERT(dev != NULL);

    if (dev->check_value != DEVICE_CHECK_VALUE)
    {
        FWLOG_ERR("Check value error: 0x%4X.", dev->check_value);
        return -1;
    }

    if (dev->close != NULL)
    {
        retval = dev->close(dev);
    }
    if (retval != 0)
    {
        FWLOG_ERR("Device <%s> closed error, return: %d.", retval);
        return retval;
    }
    dev->state &= ~DEVICE_STATE_OPENED;
    dev->state &= ~DEVICE_FLAG_MASK;
    return retval;
}

int DeviceRead(Device_t *dev, void *buf, int size)
{
    FW_ASSERT(dev != NULL);

    if (dev->check_value != DEVICE_CHECK_VALUE)
    {
        FWLOG_ERR("Check value error: 0x%4X.", dev->check_value);
        return -1;
    }

    if (dev->read != NULL)
    {
        return dev->read(dev, buf, size);
    }
    return 0;
}

int DeviceWrite(Device_t *dev, const void *buf, int size)
{
    FW_ASSERT(dev != NULL);

    if (dev->check_value != DEVICE_CHECK_VALUE)
    {
        FWLOG_ERR("Check value error: 0x%4X.", dev->check_value);
        return -1;
    }

    if (dev->write != NULL)
    {
        return dev->write(dev, buf, size);
    }
    return 0;
}

int DeviceIoctl(Device_t *dev, int cmd, void *args)
{
    FW_ASSERT(dev != NULL);

    if (dev->check_value != DEVICE_CHECK_VALUE)
    {
        FWLOG_ERR("Check value error: 0x%4X.", dev->check_value);
        return -1;
    }

    if (dev->ioctl != NULL)
    {
        return dev->ioctl(dev, cmd, args);
    }
    return 0;
}

#endif /* INCLUDE_DEVICE */
/* vim:set et ts=4 sts=4 sw=4 ft=c: */
