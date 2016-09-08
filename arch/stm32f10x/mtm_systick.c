

#include "mtm.h"
#include "stm32f10x.h"


void SysTick_Handler(void)
{
    mtm_sys_tick();
}


void mtm_systick_init(uint32_t ticks)
{
    SysTick_Config(ticks);
}

/* END OF FILE */
