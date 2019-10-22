/**
 * @file sysutils.c
 * @brief Include InitCall, FwLog and other basic modules and functions.
 * @author WANG Jun
 * @date 2017-06-02
 *
 * update:
 * 2017-09-01   WANG Jun    change function name
 * 2017-11-20   WANG Jun    add printk(), FwShowBanner(), FWLOG module.
 */

#include "frameworks.h"
#include "device.h"

#ifdef FW_ENABLE_FWLOG
/**
 * log content format:
 * time[YYYY-MM-DD hh:mm:ss], level, func: messages
 * for example:
 * 2017-05-20 13:14:01,   Error, CanOpen: Can NOT open device can0
 * 2017-07-13 00:01:12,    Info, rs485dispatch: Received 127 Bytes
 *
 * log level definition:
 * Code     Name     Severity
 *  0    Emergency   emergency, system is unusable
 *  3      Error     error conditions
 *  4     Warning    warning conditions
 *  5     Notice     normal but significate conditions
 *  6       Info     informational messages
 *  7      Debug     debug-level messages
 */

/* log level string */
static const char *kLogString[8] = {
    "EMERGENCY", "", "", "ERROR", "WARNING", "NOTICE", "INFO", "DEBUG"};

void FwLog(int level, const char *func_name, const char *fmt, ...)
{
    static char log_buffer[256];
    static int count = 0; /* TODO: 填充时间位 */
    va_list args;
    va_start(args, fmt);
    printk("%04d, %s, %s(): ", count++, kLogString[level], func_name);
    vsnprintf(log_buffer, 256 - 1, fmt, args);
    printk("%s\n", log_buffer);
    va_end(args);
}

#endif /* FW_ENABLE_FWLOG */

#ifndef CONSOLE_BUF_SIZE
#define CONSOLE_BUF_SIZE 256
#endif

static Device_t *sConsole = NULL;
void FwSetConsole(const char *name)
{
    sConsole = DeviceFindByName(name);
    if (sConsole != NULL)
    {
        if (!(DeviceGetState(sConsole) & DEVICE_STATE_OPENED))
        {
            DeviceOpen(sConsole, DEVICE_FLAG_RW | DEVICE_FLAG_RX_INT);
        }
    }
}

int kgetchar(void)
{
    unsigned char ch = 0;
    if (sConsole != NULL)
    {
        DeviceRead(sConsole, &ch, 1);
    }
    return (int)ch;
}

void kputchar(int ch)
{
    if (sConsole != NULL)
    {
        DeviceWrite(sConsole, &ch, 1);
    }
}

void kputs(const char *str)
{
    if (sConsole != NULL)
    {
        DeviceWrite(sConsole, str, strlen(str));
    }
}

void printk(const char *fmt, ...)
{
    va_list args;
    int length;
    static char console_buffer[CONSOLE_BUF_SIZE];

    va_start(args, fmt);
    length = vsnprintf(console_buffer, CONSOLE_BUF_SIZE - 1, fmt, args);
    if (length > CONSOLE_BUF_SIZE - 1)
    {
        length = CONSOLE_BUF_SIZE - 1;
    }
    if (sConsole != NULL)
    {
        DeviceWrite(sConsole, console_buffer, length);
    }
    va_end(args);
}

/* vim:set et ts=4 sts=4 sw=4 ft=c: */
