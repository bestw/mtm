
#include "mtm.h"

#ifdef MTM_SEM_ENABLE

uint8_t mtm_sem_create(struct mtm_sem_t *psem, uint16_t value)
{
    sr_type sr;

    psem = mtm_mem_get_sem();
    MTM_INTERRUPT_DISABLE();
    psem->value = value;
    mtm_list_init( &(psem->list) );
    MTM_INTERRUPT_ENABLE();

    return (MTM_ERR_NONE);
}

void mtm_sem_delete(void)
{
}

uint8_t mtm_sem_signal(struct mtm_sem_t *psem)
{
    struct mtm_task_t   *ptask;
    sr_type sr;

    MTM_INTERRUPT_DISABLE();

    if (psem->value < MTM_SEM_VALUE_MAX)
    {
        (psem->value)++;
    }

    if (mtm_list_empty(&(psem->list)) == MTM_FALSE) /* See if waiting list is empty  */
    {
        ptask = mtm_list_entry(psem->list.next, struct mtm_task_t, list);
        mtm_timer_stop(&(ptask->timer));

        mtm_list_del(&(ptask->list));           /* Delete the task from pend list  */
        ptask->pend_stat = MTM_PENDSTAT_OK;
        mtm_task_resume(ptask);
    }

    MTM_INTERRUPT_ENABLE();
    return (MTM_ERR_NONE);
}


uint8_t mtm_sem_wait(struct mtm_sem_t *psem, uint16_t timeout)
{
    sr_type sr;
    struct mtm_task_t *ptask;

    MTM_INTERRUPT_DISABLE();
    if (psem->value > 0)
    {
        psem->value--;
        MTM_INTERRUPT_ENABLE();
        return (MTM_ERR_NONE);
    }

    if (timeout == 0)
    {
        MTM_INTERRUPT_ENABLE();
        return (MTM_ERR_TIMEOUT);
    }

    ptask = mtm_cur_task;
    mtm_task_suspend(ptask);           /* Suspend the current task to wait for msg */
    ptask->pend_type = MTM_PENDTYPE_SEM;        /* Set pend type                   */
    ptask->pend_stat = MTM_PENDSTAT_OK;         /* Set pend stat                   */
    mtm_list_add2tail(&(ptask->list), &(psem->list));
    mtm_timer_create(&(ptask->timer), timeout, (mtm_timer_callback)MTM_NULL, MTM_PNULL);
    mtm_timer_start(&(ptask->timer));
    MTM_INTERRUPT_ENABLE();

    mtm_sched();

    MTM_INTERRUPT_DISABLE();
    if (psem->value > 0)
    {
        ptask->pend_stat = MTM_PENDSTAT_OK;
        psem->value--;

        MTM_INTERRUPT_ENABLE();
        return (MTM_ERR_NONE);
    }

    ptask->pend_stat = MTM_PENDSTAT_TIMEOUT;
    MTM_INTERRUPT_ENABLE();
    return (MTM_ERR_TIMEOUT);
}


uint8_t mtm_sem_try(struct mtm_sem_t *psem)
{
    uint8_t error;

    error = mtm_sem_wait(psem, 0);
    return (error);
}


#endif /* MTM_SEM_ENABLE */

/* END OF FILE */
