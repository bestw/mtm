/**
 * @file shell.c
 * @brief 
 * @author WANG Jun
 * @date 2017-11-21
 *
 * update:
 */

#include "frameworks.h"
#include "shell.h"
#include "FreeRTOS.h"
#include "task.h"

extern void DeviceList(int argc, char **argv);

tinysh_cmd_t sCmdDevList = {NULL, "device", "list devices", "<cr>",
                            DeviceList, 0, 0, 0};

static void CmdFuncTaskList(int argc, char **argv)
{
    uint8_t list_buffer[512];

    printk(" TaskName     State  Priority StackFree Index\n");
    printk("============ ======= ======== ========= =====\n");
    vTaskList((char *)&list_buffer);
    printk("%s\n", list_buffer);
    printk("State: 'B' Block, 'R' Ready, 'D' Deleted, 'S' Suspend.\n");
    printk("Stack space unit is word = 4 bytes.\n");

    printk("\n TaskName      RunCount      UsageRate\n");
    printk("============  ============  ===========\n");
    vTaskGetRunTimeStats((char *)&list_buffer);
    printk("%s\n", list_buffer);
}
tinysh_cmd_t sCmdTaskList = {NULL, "task", "list tasks", "<cr>",
                             CmdFuncTaskList, 0, 0, 0};

static void CmdFuncFree(int argc, char **argv)
{
    size_t free_size = xPortGetFreeHeapSize();
    printk("剩余内存: %d   %.1fK\n", free_size, free_size / 1024.0);
}
tinysh_cmd_t sCmdFree = {NULL, "free", "show the memory usage", "<cr>",
                         CmdFuncFree, 0, 0, 0};

static void CmdFuncVersion(int argc, char **argv)
{
    FwShowBanner();
}
tinysh_cmd_t sCmdVersion = {NULL, "version", "show banner and version", "<cr>",
                            CmdFuncVersion, 0, 0, 0};

int ShellMain(void)
{
    int c;
    int again = 1, count = 0;

    tinysh_add_command(&sCmdDevList);
    tinysh_add_command(&sCmdTaskList);
    tinysh_add_command(&sCmdFree);
    tinysh_add_command(&sCmdVersion);

    tinysh_char_in('\n');
    /* main loop */
    while (again)
    {
        c = kgetchar();
        if (c)
        {
            count = 0;
            tinysh_char_in((unsigned char)c);
        }

        if (count++ > 10)
        {
            vTaskDelay(5);
        }
    }
    printk("\nBye\n");
    return 0;
}

/******************************************************************************/

inline void tinysh_char_out(unsigned char c)
{
    kputchar((int)c);
}

inline void tinysh_puts(char *s)
{
    kputs(s);
}

/* vim:set et ts=4 sts=4 sw=4 ft=c: */
