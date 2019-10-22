
#include <math.h>
#include "frameworks.h"
#include "device.h"

static uint32_t AdcFilterMean(uint32_t *buffer, int length)
{
    double sum;
    uint32_t max, min;
    int i;

    FW_ASSERT(buffer != NULL);
    FW_ASSERT(length > 2);

    max = buffer[0];
    min = buffer[0];
    sum = 0.0;
    for (i = 0; i < length; i++)
    {
        sum += buffer[i];
        max = max < buffer[i] ? buffer[i] : max;
        min = min > buffer[i] ? buffer[i] : min;
    }

    return (uint32_t)((sum - max - min) / (length - 2));
}

static uint32_t AdcFilterRMS(uint32_t *buffer, int length)
{
    double sum;
    uint32_t max, min;
    int i;

    FW_ASSERT(buffer != NULL);
    FW_ASSERT(length > 2);

    max = buffer[0];
    min = buffer[0];
    sum = 0.0;
    for (i = 0; i < length; i++)
    {
        sum += buffer[i] * buffer[i];
        max = max < buffer[i] ? buffer[i] : max;
        min = min > buffer[i] ? buffer[i] : min;
    }

    return (uint32_t)sqrt((sum - max * max - min * min) / (length - 2));
}

static int AdcInit()
{
    ;
}

static int AdcRelease()
{
    ;
}

static int AdcRead()
{
    ;
}

static int AdcIoCtl()
{
    return 0;
}

int AdcInstall(void)
{
    Device_t *dev = NULL;

    dev->init    = AdcInit;
    dev->release = AdcRelease;
    dev->open    = NULL;
    dev->close   = NULL;
    dev->read    = AdcRead;
    dev->write   = NULL;
    dev->ioctl   = AdcIoCtl;

    dev->data = NULL;
    return DeviceInstall(dev, "ad7991", 0);
}
