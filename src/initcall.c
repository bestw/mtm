/**
 * @file initcall.c
 * @brief 
 * @author WANG Jun
 * @date 2017-12-12
 *
 * update:
 */
#include "frameworks.h"
#include "FreeRTOS.h"
#include "task.h"

static int InitCallStart(void)
{
    return 0;
}
_INITCALL(InitCallStart, "0");

static int InitCallBoardEnd(void)
{
    return 0;
}
_INITCALL(InitCallBoardEnd, "1.end");

static int InitCallEnd(void)
{
    return 0;
}
_INITCALL(InitCallEnd, "6");

int DoBoardInitCall(void)
{
    const InitCallFunc_t *init_function;

    for (init_function = (const InitCallFunc_t *)&InitCallStart;
         init_function < (const InitCallFunc_t *)&InitCallBoardEnd; init_function++)
    {
        (*init_function)();
    }

    return 0;
}

int DoInitCall(void)
{
    const InitCallFunc_t *init_function;

    for (init_function = (const InitCallFunc_t *)&InitCallBoardEnd;
         init_function < (const InitCallFunc_t *)&InitCallEnd; init_function++)
    {
        (*init_function)();
    }

    return 0;
}

void InitTaskEntry(void *param)
{
    (void)param;
    DoInitCall();
    FwShowBanner();
    vTaskDelete(NULL);
}

extern void BoardInit(void);

void FrameworksInit(void)
{
    BoardInit();
    xTaskCreate(InitTaskEntry, "t_init", 2048, NULL, configMAX_PRIORITIES - 1, NULL);
}

/* vim:set et ts=4 sts=4 sw=4 ft=c: */
