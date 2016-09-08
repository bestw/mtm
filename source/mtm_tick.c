
#include "mtm.h"

static volatile uint32_t    mtm_cur_time; /* System current tick time */

void mtm_time_init(void)
{
    sr_type sr;

    MTM_INTERRUPT_DISABLE();
    mtm_cur_time = 0ul;
    MTM_INTERRUPT_ENABLE();
}

uint32_t mtm_time_elapse(void)
{
    return (mtm_cur_time);
}

/**
 * @brief This function is called by system tick interrupt handler.
 */
void mtm_sys_tick(void)
{
    struct mtm_task_t *ptask;
    sr_type sr;

    MTM_INTERRUPT_DISABLE();
    mtm_cur_time++;

    ptask = mtm_cur_task;
    ptask->tick--;

    if (ptask->tick == 0)
    {
        ptask->tick = ptask->orig_tick;
        mtm_task_yield();
    }

    if (mtm_timer_manage())
    {
        mtm_sched();
    }
    MTM_INTERRUPT_ENABLE();
}

/* END OF FILE */
