
#include "mtm.h"

#ifdef MTM_MQUEUE_ENABLE

uint8_t mtm_mqueue_create(struct mtm_mqueue_t *pmq, void **msg_start, uint16_t msg_nbr)
{
    pmq = mtm_mem_get_mqueue();

    if (pmq != (struct mtm_mqueue_t *)0)
    {
        mtm_list_init(&(pmq->list));
        pmq->head     = msg_start;
        pmq->tail     = &msg_start[msg_nbr - 1];
        pmq->in       = msg_start;
        pmq->out      = msg_start;
        pmq->msg_nbr  = msg_nbr;
        pmq->entries  = 0;
    }

    return (MTM_ERR_NONE);
}

void mtm_mqueue_delete(void)
{
}

/**
 * @brief This function sends a message to a queue.
 *
 * @param pmq
 * @param pmsg
 *
 * @return 
 */
uint8_t mtm_mqueue_send(struct mtm_mqueue_t *pmq, void *pmsg)
{
    sr_type sr;
    struct mtm_task_t   *ptask;

    MTM_INTERRUPT_DISABLE();
    if (pmq->entries >= pmq->msg_nbr)
    {
        MTM_INTERRUPT_ENABLE();
        return (MTM_ERR_MQUEUE_FULL);
    }

    *pmq->in = pmsg;
    pmq->entries++;
    if (pmq->in == pmq->tail)
    {
        pmq->in = pmq->head;
    }
    else
    {
        pmq->in++;
    }

    if (mtm_list_empty(&(pmq->list)) == MTM_FALSE) /* See if exist task waiting msg */
    {
        ptask = mtm_list_entry(pmq->list.next, struct mtm_task_t, list);
        
        mtm_timer_stop(&(ptask->timer));

        mtm_list_del(pmq->list.next);
        ptask->pend_stat = MTM_PENDSTAT_OK;
        mtm_task_resume(ptask);
        MTM_INTERRUPT_ENABLE();

        mtm_sched();
        MTM_INTERRUPT_DISABLE();
    }

    MTM_INTERRUPT_ENABLE();
    return (MTM_ERR_NONE);
}


uint8_t mtm_mqueue_receive(struct mtm_mqueue_t *pmq, void *pmsg, uint16_t timeout)
{
    sr_type sr;
    struct mtm_task_t   *ptask;

    MTM_INTERRUPT_DISABLE();
    if (pmsg == (void *)0);
    if (pmq->entries > 0)
    {
        pmsg = *pmq->out;
        pmq->entries--;
        if (pmq->out == pmq->tail)
        {
            pmq->out = pmq->head;
        }
        else
        {
            pmq->out++;
        }
        MTM_INTERRUPT_ENABLE();
        return (MTM_PENDSTAT_OK);
    }

    if (timeout == 0)   /* Wait for a message */
    {
        MTM_INTERRUPT_ENABLE();
        return (MTM_PENDSTAT_TIMEOUT);
    }

    ptask  = mtm_cur_task;
    mtm_task_suspend(ptask);
    ptask->pend_type = MTM_PENDTYPE_MQUEUE;
    ptask->pend_stat = MTM_PENDSTAT_OK;

    mtm_list_add2tail(&(ptask->list), &(pmq->list));
    mtm_timer_create(&(ptask->timer), timeout, (mtm_timer_callback)MTM_NULL, (void *)MTM_NULL);
    mtm_timer_start(&(ptask->timer));
    MTM_INTERRUPT_ENABLE();

    mtm_sched();
    MTM_INTERRUPT_DISABLE();

    if (pmq->entries > 0)
    {
        pmsg = *pmq->out;
        pmq->entries--;
        if (pmq->out == pmq->tail)
        {
            pmq->out = pmq->head;
        }
        else
        {
            pmq->out++;
        }
        ptask->pend_stat = MTM_PENDSTAT_OK;

        MTM_INTERRUPT_ENABLE();
        return (MTM_ERR_NONE);
    }

    pmsg = (void *)0;
    ptask->pend_stat = MTM_PENDSTAT_TIMEOUT;

    MTM_INTERRUPT_ENABLE();
    return (MTM_ERR_TIMEOUT);
}


uint8_t mtm_mqueue_try(struct mtm_mqueue_t *pmq, void *pmsg)
{ 
    uint8_t error;

    error = mtm_mqueue_receive(pmq, pmsg, 0);
    return (error);
}

#endif /* MTM_MQUEUE_ENABLE */

/* END OF FILE */
