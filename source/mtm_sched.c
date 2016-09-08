
#include "mtm.h"

/**
 * Global variable defines
 */
volatile uint8_t     mtm_sched_nesting; /* Schedule nesting lock             */
volatile uint8_t     mtm_inter_nesting; /* Interrupt nesting lock            */

volatile uint8_t     mtm_cur_prio;      /* Current running priority          */
volatile uint8_t     mtm_ready_mask;    /* Ready priorities                  */
volatile uint8_t     mtm_highest_prio;  /* Current highest ready priority    */

struct mtm_list_t   mtm_ready_list[MTM_LOWEST_PRIO + 1]; /* Ready list of each prio */
struct mtm_task_t*  mtm_cur_task;       /* Current running task              */
struct mtm_task_t*  mtm_next_task;      /* Next task                         */

const uint8_t   mtm_unbitmap[256] = {
    0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0x00 to 0x0F */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0x10 to 0x1F */
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0x20 to 0x2F */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0x30 to 0x3F */
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0x40 to 0x4F */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0x50 to 0x5F */
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0x60 to 0x6F */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0x70 to 0x7F */
    7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0x80 to 0x8F */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0x90 to 0x9F */
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0xA0 to 0xAF */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0xB0 to 0xBF */
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0xC0 to 0xCF */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0xD0 to 0xDF */
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, /* 0xE0 to 0xEF */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0  /* 0xF0 to 0xFF */
};

void mtm_init(void)
{
    mtm_sched_init();
    mtm_mem_init();
    mtm_time_init();
    mtm_timer_init();
    mtm_idle_task_init();
}

void mtm_int_enter(void)
{
    if (mtm_inter_nesting < 255u)
    {
        mtm_inter_nesting++;
    }
}

void mtm_int_exit(void)
{
    sr_type sr;

    MTM_INTERRUPT_DISABLE();
    if (mtm_inter_nesting > 0u)
    {
        mtm_inter_nesting--;
    }
    if (mtm_inter_nesting == 0u)
    {
        if (mtm_sched_nesting == 0u)
        {
            mtm_sched();
        }
    }
    MTM_INTERRUPT_ENABLE();
}

void mtm_sched_lock(void)
{
    sr_type sr;

    MTM_INTERRUPT_DISABLE();
    if (mtm_inter_nesting == 0u)
    {
        if (mtm_sched_nesting < 255u)
        {
            mtm_sched_nesting++;
        }
    }
    MTM_INTERRUPT_ENABLE();
}

void mtm_sched_unlock(void)
{
    sr_type sr;

    MTM_INTERRUPT_DISABLE();
    if (mtm_inter_nesting == 0u)
    {
        if (mtm_sched_nesting > 0u)
        {
            mtm_sched_nesting--;
            if (mtm_sched_nesting == 0u)
            {
                mtm_sched();
            }
        }
    }
    MTM_INTERRUPT_ENABLE();
}

void mtm_start(void)
{
    sr_type sr;

    MTM_INTERRUPT_DISABLE();
    mtm_systick_init(720000);/* 10ms */
    mtm_get_highest_prio();
    mtm_cur_prio = mtm_highest_prio;
    mtm_next_task = mtm_list_entry(mtm_ready_list[mtm_highest_prio].next,
                                   struct mtm_task_t,
                                   list);
    mtm_cur_task = mtm_next_task;
    MTM_INTERRUPT_ENABLE();
    mtm_start_first();

    while(1)
    {
    }
}

void mtm_sched(void)
{
    sr_type sr;

    MTM_INTERRUPT_DISABLE();
    if (mtm_sched_nesting == 0u)
    {
        if (mtm_inter_nesting == 0u)
        {
            mtm_get_highest_prio();
            mtm_next_task = mtm_list_entry(mtm_ready_list[mtm_highest_prio].next,
                                           struct mtm_task_t,
                                           list);
            if (mtm_next_task != mtm_cur_task) /* If cur task isn't the next task */
            {
                mtm_context_switch();
            }
        }
    }
    MTM_INTERRUPT_ENABLE();
}


void mtm_get_highest_prio(void)
{
    mtm_highest_prio = mtm_unbitmap[mtm_ready_mask];
}

void mtm_add_to_readylist(struct mtm_task_t *ptask)
{
    sr_type sr;

    MTM_INTERRUPT_DISABLE();
    ptask->stat = MTM_STAT_READY;
    mtm_list_del(&(ptask->list));
    mtm_list_add2tail(&(ptask->list), &(mtm_ready_list[ptask->prio]));
    mtm_ready_mask |= ((uint8_t)1u << ptask->prio);
    MTM_INTERRUPT_ENABLE();
}

void mtm_add_to_readylist_head(struct mtm_task_t *ptask)
{
    sr_type sr;

    MTM_INTERRUPT_DISABLE();
    ptask->stat = MTM_STAT_READY;
    mtm_list_del(&(ptask->list));
    mtm_list_add2head(&(ptask->list), &(mtm_ready_list[ptask->prio]));
    mtm_ready_mask |= ((uint8_t)1u << ptask->prio);
    MTM_INTERRUPT_ENABLE();
}

void mtm_remove_from_readylist(struct mtm_task_t *ptask)
{
    sr_type sr;

    MTM_INTERRUPT_DISABLE();
    mtm_list_del(&(ptask->list));
    if (mtm_list_empty(&(mtm_ready_list[ptask->prio])) == MTM_TRUE)
    {
        mtm_ready_mask &= ~((uint8_t)1u << ptask->prio);
    }
    MTM_INTERRUPT_ENABLE();
}


void mtm_sched_chkstk(struct mtm_task_t *ptask)
{
}

void mtm_sched_init(void)
{
    sr_type sr;
    uint8_t tmp;

    MTM_INTERRUPT_DISABLE();
    mtm_sched_nesting = 0u;
    mtm_inter_nesting = 0u;
    mtm_cur_prio = MTM_LOWEST_PRIO;
    mtm_ready_mask = 0u;
    mtm_highest_prio = MTM_LOWEST_PRIO;

    for (tmp = 0; tmp < MTM_LOWEST_PRIO + 1; tmp++)
    {
        mtm_list_init( &(mtm_ready_list[tmp]) );
    }
    mtm_cur_task  = (struct mtm_task_t *)0;
    mtm_next_task = (struct mtm_task_t *)0;
    MTM_INTERRUPT_ENABLE();
}


/* END OF FILE */
