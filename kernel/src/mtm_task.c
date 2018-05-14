
#include "mtm.h"

/**
 * Global variable defines
 */
#define MTM_IDLE_STACK_SIZE     128u                 /* Idle task stack size */
static stack_t  mtm_idle_stack[MTM_IDLE_STACK_SIZE]; /* Idle task stack      */


/**
 * @brief   Create the task.
 *
 * @param ptask         point to task control block
 * @param task_entry
 * @param param
 * @param stk_addr      point to the task's bottom of stack.
 * @param stk_size      size of stack
 * @param prio          task's priority
 * @param tick          task running tick number
 * @param pname         task name pointer
 *
 * @return  ptask     A pionter to task control block
 */
struct mtm_task_t * mtm_task_create(void              (*task_entry)(void *param),
                                    void              *param,
                                    stack_t           *stk_addr,
                                    uint32_t          stk_size,
                                    uint8_t           prio,
                                    uint16_t          tick)
{
    struct mtm_task_t * ptask; 
    ptask = mtm_mem_get();                                  /* Get a free task control block */

    ptask->stk = mtm_stack_init(task_entry, param,
                                &stk_addr[stk_size - 1], (void *)mtm_task_return);
    mtm_list_init(&(ptask->list));

    ptask->orig_prio = prio;
    ptask->prio = prio;

    ptask->stat = MTM_STAT_DORMANT;
    ptask->pend_type = MTM_PENDTYPE_NONE;
    ptask->pend_stat = MTM_PENDSTAT_OK;

    ptask->orig_tick = tick;
    ptask->tick      = tick;
    mtm_timer_destroy(&(ptask->timer));

    return (ptask);
}

void mtm_task_return(void)
{
    struct mtm_task_t *ptask;
    sr_type sr;

    MTM_INTERRUPT_DISABLE();
    ptask = mtm_cur_task;
    mtm_task_destroy(ptask);
    MTM_INTERRUPT_ENABLE();
}

void mtm_task_start(struct mtm_task_t *ptask)
{
    ptask->stat = MTM_STAT_SUSPEND;
    mtm_add_to_readylist(ptask);
}

void mtm_task_destroy(struct mtm_task_t *ptask)
{
    mtm_remove_from_readylist(ptask);
    ptask->stat = MTM_STAT_DORMANT;
    mtm_sched();
}

void mtm_task_yield(void)
{
    struct mtm_task_t *ptask;
    sr_type sr;

    MTM_INTERRUPT_DISABLE();
    ptask = mtm_cur_task;

    if ( (ptask->stat == MTM_STAT_READY) && (ptask->list.prev != ptask->list.next) )
    {
        mtm_list_del( &(ptask->list) );
        mtm_list_add2tail( &(ptask->list), &mtm_ready_list[ptask->prio] );
        mtm_sched();
    }
    MTM_INTERRUPT_ENABLE();
}

void mtm_task_suspend(struct mtm_task_t *ptask)
{
    ptask->stat = MTM_STAT_SUSPEND;
    mtm_remove_from_readylist(ptask);
}

void mtm_task_resume(struct mtm_task_t *ptask)
{
    mtm_add_to_readylist(ptask);
}

void mtm_task_resume_head(struct mtm_task_t *ptask)
{
    mtm_add_to_readylist_head(ptask);
}


/**
 * @brief change the task's running priority
 *
 * @param ptask point to task which priority will be changed
 * @param prio  priority number
 */
void mtm_task_change_prio(struct mtm_task_t *ptask, uint8_t prio)
{
    ptask->prio = prio;
}

/**
 * @brief Delay the current task for several ticks
 *
 * @param tick  tick to delay
 */
void mtm_task_delay(tick_t tick)
{
    struct mtm_task_t   *ptask;
    sr_type sr;

    MTM_INTERRUPT_DISABLE();
    ptask = mtm_cur_task;

    mtm_task_suspend(ptask);
    ptask->pend_type = MTM_PENDTYPE_DELAY;
    ptask->pend_stat = MTM_PENDSTAT_OK;

    mtm_timer_create(&(ptask->timer), tick, (mtm_timer_callback)MTM_NULL, MTM_PNULL);
    mtm_timer_start(&(ptask->timer));
    MTM_INTERRUPT_ENABLE();

    mtm_sched();
}

uint8_t mtm_task_delay_resume(struct mtm_task_t *ptask)
{
    sr_type sr;

    MTM_INTERRUPT_DISABLE();
    if (ptask->pend_type != MTM_PENDTYPE_DELAY)
    {
        MTM_INTERRUPT_ENABLE();
        return (MTM_ERR_PENDTYPE);
    }

    mtm_timer_stop(&(ptask->timer));
    ptask->pend_stat = MTM_PENDSTAT_ABORT;

    mtm_list_del(&(ptask->list));                           /* Delete the task from pend list */
    mtm_task_resume(ptask);
    MTM_INTERRUPT_ENABLE();

    mtm_sched();
    return (MTM_ERR_NONE);
}

void mtm_idle_task_init(void)
{
    struct mtm_task_t *ptask;

    ptask = mtm_task_create(&mtm_idle_task, (void *)0,
                    &mtm_idle_stack[0], MTM_IDLE_STACK_SIZE, MTM_LOWEST_PRIO, 5u);
    mtm_task_start(ptask);
}

void mtm_idle_task(void *param)
{
    while (MTM_TRUE)
    {
        param = param;
    }
}

/* END OF FILE */
