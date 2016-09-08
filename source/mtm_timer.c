
#include "mtm.h"

static struct mtm_list_t    mtm_timer_list; /* Running timer list */

void mtm_timer_init(void)
{
    sr_type sr;

    MTM_INTERRUPT_DISABLE();
    mtm_list_init(&mtm_timer_list);
    MTM_INTERRUPT_ENABLE();
}

uint8_t mtm_timer_create(struct mtm_timer_t *ptimer, tick_t cnt, mtm_timer_callback callback, void *param)
{
    mtm_list_init(&(ptimer->list));
    ptimer->callback    = callback;
    ptimer->cb_param    = param;
    ptimer->count       = cnt;
    ptimer->compare     = 0u;
    ptimer->stat        = MTM_TMRSTAT_STOPPED;

    return (ptimer->stat);
}

uint8_t mtm_timer_destroy(struct mtm_timer_t *ptimer)
{
    mtm_list_init(&(ptimer->list));
    ptimer->callback    = (mtm_timer_callback)MTM_NULL;
    ptimer->cb_param    = MTM_PNULL;
    ptimer->count       = 0u;
    ptimer->compare     = 0u;
    ptimer->stat        = MTM_TMRSTAT_NOT_USED;

    return (ptimer->stat);
}

uint8_t mtm_timer_start(struct mtm_timer_t *ptimer)
{
    sr_type sr;
    tick_t cur_time;
    
    MTM_INTERRUPT_DISABLE();
    cur_time = mtm_time_elapse();
    ptimer->compare = ptimer->count + cur_time;
    ptimer->stat    = MTM_TMRSTAT_RUNNING;
    mtm_list_del(&(ptimer->list));
    mtm_list_add2tail(&(ptimer->list), &mtm_timer_list);
    MTM_INTERRUPT_ENABLE();

    return (ptimer->stat);
}

/**
 * @brief Stop the timer
 *
 * @param ptimer    point to timer wanted to be stopped
 *
 * @return  MTM_TMRSTAT_STOPPED     time's up
 *          MTM_TMRSTAT_ABORT       timer has been aborted
 */
uint8_t mtm_timer_stop(struct mtm_timer_t *ptimer)
{
    sr_type sr;
    tick_t cur_time;

    MTM_INTERRUPT_DISABLE();
    cur_time = mtm_time_elapse();
    if (ptimer->stat == MTM_TMRSTAT_RUNNING)
    {
        if (ptimer->compare <= cur_time)
        {
            ptimer->stat = MTM_TMRSTAT_STOPPED;                   /* Timer has been stopped       */
            if (ptimer->callback != (mtm_timer_callback)MTM_NULL) /* See if has callback funciton */
            {
                (ptimer->callback)(ptimer->cb_param);
            }
        }
        else
        {
            ptimer->stat = MTM_TMRSTAT_ABORT;                     /* Timer has been aborted       */
        }
        mtm_list_del(&(ptimer->list));
        MTM_INTERRUPT_ENABLE();
    }

    return (ptimer->stat);
}

tick_t mtm_timer_get_remain(struct mtm_timer_t *ptimer)
{
    return (ptimer->compare - mtm_time_elapse());
}


/**
 * @brief Soft timer manager, invoked by mtm_sys_tick.
 *
 * @return to_flag: timeout flag
 */
uint8_t mtm_timer_manage(void)
{
    struct mtm_list_t   *plist;
    struct mtm_timer_t  *ptimer;
    struct mtm_task_t   *ptask;
    uint8_t             to_flag = 0u; /* Time out flag */

    if (mtm_list_empty(&mtm_timer_list) == MTM_FALSE)
    {
        plist = mtm_timer_list.next;
        while (plist != &mtm_timer_list)
        {
            ptimer = mtm_list_entry(plist, struct mtm_timer_t, list);
            plist = plist->next;
            if (ptimer->compare <= mtm_time_elapse())   /* See if timer is timeout */
            {
                mtm_timer_stop(ptimer);
                ptask = mtm_list_entry(ptimer, struct mtm_task_t, timer);
                ptask->pend_stat = MTM_PENDSTAT_TIMEOUT;
                mtm_task_resume(ptask);
                to_flag = 1u;
            }
        } /* end of while */
    }
    return (to_flag);
}

/* END OF FILE */
