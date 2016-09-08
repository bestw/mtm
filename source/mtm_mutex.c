
#include "mtm.h"

#ifdef MTM_MUTEX_ENABLE

/* Mutex flag */
#define MTM_MUTEX_AVAILABLE         1
#define MTM_MUTEX_NOT_AVIALABLE     0

uint8_t mtm_mutex_create(struct mtm_mutex_t *pmutex)
{
    sr_type sr;

    pmutex = mtm_mem_get_mutex();
    MTM_INTERRUPT_DISABLE();
    pmutex->value = MTM_MUTEX_AVAILABLE;
    pmutex->hold_prio = 0xFFu;
    pmutex->owner = (struct mtm_task_t *)0;
    mtm_list_init( &(pmutex->list) );
    MTM_INTERRUPT_ENABLE();

    return (MTM_ERR_NONE);
}

void mtm_mutex_destroy(void)
{
    sr_type sr;
    MTM_INTERRUPT_DISABLE();
    MTM_INTERRUPT_ENABLE();
}

uint8_t mtm_mutex_signal(struct mtm_mutex_t *pmutex)
{
    struct mtm_task_t   *ptask;
    sr_type sr;

    MTM_INTERRUPT_DISABLE();
    ptask = mtm_cur_task;

    if (ptask != pmutex->owner)
    {
        MTM_INTERRUPT_ENABLE();
        return (MTM_ERR_NOT_MUTEX_OWNER);
    }

    pmutex->value = MTM_MUTEX_AVAILABLE;
    pmutex->hold_prio = 0xFF;
    pmutex->owner = (struct mtm_task_t *)MTM_NULL;

    if (mtm_list_empty(&(pmutex->list)) == MTM_FALSE)
    {
        ptask = mtm_list_entry(pmutex->list.next, struct mtm_task_t, list);
        mtm_timer_stop(&(ptask->timer));

        mtm_list_del(&(ptask->list));           /* Delete the task from pend list  */
        ptask->pend_stat = MTM_PENDSTAT_OK;
        mtm_task_resume(ptask);
    }
    MTM_INTERRUPT_ENABLE();

    return (MTM_ERR_NONE);
}

uint8_t mtm_mutex_wait(struct mtm_mutex_t *pmutex, uint32_t timeout)
{
    struct mtm_task_t *ptask;
    sr_type sr;

    MTM_INTERRUPT_DISABLE();
    ptask = mtm_cur_task;
    if (pmutex->owner == ptask)
    {
        MTM_INTERRUPT_ENABLE();
        return (MTM_ERR_NONE);
    }

    if (pmutex->value == MTM_MUTEX_AVAILABLE)   /* Mutex is available                */
    {
        pmutex->value = MTM_MUTEX_NOT_AVIALABLE;
        pmutex->owner = ptask;
        pmutex->hold_prio = ptask->prio;

        MTM_INTERRUPT_ENABLE();
        return (MTM_ERR_NONE);
    }

    if (timeout == 0)
    {
        MTM_INTERRUPT_ENABLE();
        return (MTM_ERR_TIMEOUT);
    }

    /* See if the mutex holder's prio is lower than current task */
    if (ptask->prio < pmutex->hold_prio)     /* Number lower, prio higher            */
    {
        mtm_task_suspend(pmutex->owner);
        mtm_task_change_prio(pmutex->owner, ptask->prio); /* Change owner's prio     */
        mtm_task_resume_head(pmutex->owner); /* Let owner run first                  */
    }

    /* Sleep the task for waiting mutex */
    mtm_task_suspend(ptask);
    ptask->pend_type = MTM_PENDTYPE_MUTEX;
    ptask->pend_stat = MTM_PENDSTAT_OK;
    mtm_list_add2tail(&(ptask->list), &(pmutex->list)); /* Add to mutex waiting list */

    mtm_timer_create(&(ptask->timer), timeout, (mtm_timer_callback)MTM_NULL, MTM_PNULL);
    mtm_timer_start(&(ptask->timer));
    MTM_INTERRUPT_ENABLE();

    mtm_sched();
    MTM_INTERRUPT_DISABLE();
    if (pmutex->value == MTM_MUTEX_AVAILABLE)   /* Mutex is available                */
    {
        ptask->pend_stat = MTM_PENDSTAT_OK;

        pmutex->value = MTM_MUTEX_NOT_AVIALABLE;
        pmutex->owner = ptask;
        pmutex->hold_prio = ptask->prio;

        MTM_INTERRUPT_ENABLE();
        return (MTM_ERR_NONE);
    }

    ptask->pend_stat = MTM_PENDSTAT_TIMEOUT;
    MTM_INTERRUPT_ENABLE();
    return (MTM_ERR_TIMEOUT);
}


uint8_t mtm_mutex_try(struct mtm_mutex_t *pmutex)
{
    uint8_t error;

    error = mtm_mutex_wait(pmutex, 0);
    return (error);
}


#endif /* MTM_MUTEX_ENABLE */

/* END OF FILE */
