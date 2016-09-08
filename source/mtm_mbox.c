
#include "mtm.h"

#ifdef MTM_MBOX_ENABLE

uint8_t mtm_mbox_create(struct mtm_mbox_t *pmbox, void *pmsg)
{
    pmbox = mtm_mem_get_mbox();

    if (pmbox != (struct mtm_mbox_t *)0)
    {
        mtm_list_init(&(pmbox->mb_list));
    }
    pmbox->mb_msg = pmsg;

    return (MTM_ERR_NONE);
}


void mtm_mbox_delete(void)
{
}


/**
 * @brief   Send a message to the mailbox.
 *
 * @param pmbox     Piont to the mailbox
 * @param pmsg      The message pionter
 *
 * @return  MTM_ERR_MBOX_FULL   The mailbox is full
 *          MTM_ERR_NONE        Send message successfully
 */
uint8_t mtm_mbox_send(struct mtm_mbox_t *pmbox, void *pmsg)
{
    struct mtm_task_t   *ptask;
    sr_type sr;

    MTM_INTERRUPT_DISABLE();
    if ((pmbox->mb_msg) != (void *)0)           /* See if the mailbox is full      */
    {
        MTM_INTERRUPT_ENABLE();
        return (MTM_ERR_MBOX_FULL);
    }

    pmbox->mb_msg = pmsg;                       /* Send the message to the mailbox */

    if (mtm_list_empty(&(pmbox->mb_list)) == MTM_FALSE) /* See if waiting list is empty */
    {
        ptask = mtm_list_entry(pmbox->mb_list.next, struct mtm_task_t, list);

        mtm_timer_stop(&(ptask->timer));        /* Stop timer to delay             */

        mtm_list_del(&(ptask->list));           /* Delete the task from pend list  */
        ptask->pend_stat = MTM_PENDSTAT_OK;
        mtm_task_resume(ptask);

        MTM_INTERRUPT_ENABLE();

        mtm_sched();
        MTM_INTERRUPT_DISABLE();
    }
    MTM_INTERRUPT_ENABLE();
    return (MTM_ERR_NONE);
}


/**
 * @brief   Receive a message from the mailbox. If not available, waiting for several ticks.
 *
 * @param pmbox     Piont to mailbox
 * @param pmsg      Piont to a message container to store the received message
 * @param timeout   Waiting ticks if does not have a message in the mailbox
 *
 * @return  MTM_ERR_NONE    Receive message successfully
 *          MTM_ERR_TIMEOUT Timeout of waiting message
 */
uint8_t mtm_mbox_receive(struct mtm_mbox_t *pmbox, void *pmsg, uint16_t timeout)
{
    sr_type sr;
    struct mtm_task_t   *ptask;

    pmsg = pmsg;

    MTM_INTERRUPT_DISABLE();
    if (pmbox->mb_msg != (void *)0)             /* See if has a msg in mailbox     */
    {
        pmsg = pmbox->mb_msg;                   /* Receive a message from mailbox  */
        pmbox->mb_msg = (void *)0;              /* Clear the mailbox's message     */
        MTM_INTERRUPT_ENABLE();
        return (MTM_ERR_NONE);
    }

    if (timeout == 0)                           /* See if waiting for the message  */
    {
        MTM_INTERRUPT_ENABLE();
        return (MTM_ERR_TIMEOUT);
    }

    /* Wait for a message */
    ptask  = mtm_cur_task;
    mtm_task_suspend(ptask);           /* Suspend the current task to wait for msg */
    ptask->pend_type = MTM_PENDTYPE_MBOX;       /* Set pend type                   */
    ptask->pend_stat = MTM_PENDSTAT_OK;         /* Set pend stat                   */

    mtm_list_add2tail(&(ptask->list), &(pmbox->mb_list)); /* Add task to mbox pending list */
    mtm_timer_create(&(ptask->timer), timeout, (mtm_timer_callback)MTM_NULL, MTM_PNULL);
    mtm_timer_start(&(ptask->timer));
    MTM_INTERRUPT_ENABLE();

    mtm_sched();
    MTM_INTERRUPT_DISABLE();

    /* See if the message is available */
    if (pmbox->mb_msg != (void *)0)
    {
        ptask->pend_stat = MTM_PENDSTAT_OK;
        pmsg = pmbox->mb_msg;                   /* Receive a message from mailbox  */
        pmbox->mb_msg = (void *)0;              /* Clear the mailbox's message     */

        MTM_INTERRUPT_ENABLE();
        return (MTM_ERR_NONE);
    }
    else
    {
        ptask->pend_stat = MTM_PENDSTAT_TIMEOUT;
        pmsg = (void *)0;                       /* Message isn't available         */
    }

    MTM_INTERRUPT_ENABLE();

    return (MTM_ERR_TIMEOUT);
}

/**
 * @brief   Try to receive a message from the mailbox. 
 *
 * @param pmbox     Piont to mailbox
 * @param pmsg      Piont to a message container to store the received message
 *
 * @return  MTM_ERR_NONE    
 */
uint8_t mtm_mbox_try(struct mtm_mbox_t *pmbox, void *pmsg)
{
    uint8_t error;

    error = mtm_mbox_receive(pmbox, pmsg, 0);

    return (error);
}

#endif /* MTM_MBOX_ENABLE */

/* END OF FILE */
