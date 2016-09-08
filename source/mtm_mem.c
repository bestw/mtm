
#include "mtm.h"

/**
 * Global variable defines
 */
struct mtm_task_t   mtm_task_table[MTM_MAX_TASK_NBR];
uint8_t             mtm_task_free;

#ifdef MTM_MUTEX_ENABLE
struct mtm_mutex_t  mtm_mutex_table[MTM_MAX_MUTEX_NBR];
uint8_t             mtm_mutex_free;
#endif

#ifdef MTM_SEM_ENABLE
struct mtm_sem_t    mtm_sem_table[MTM_MAX_SEM_NBR];
uint8_t             mtm_sem_free;
#endif

#ifdef MTM_MBOX_ENABLE
struct mtm_mbox_t   mtm_mbox_table[MTM_MAX_MBOX_NBR];
uint8_t             mtm_mbox_free;
#endif

#ifdef MTM_MQUEUE_ENABLE
struct mtm_mqueue_t mtm_mqueue_table[MTM_MAX_MQUEUE_NBR];
uint8_t             mtm_mqueue_free;
#endif

void mtm_mem_copy(uint8_t *dst, const uint8_t *src, uint16_t size)
{
    while (size > 0)
    {
        *dst++ = *src++;
        size--;
    }
}

void mtm_mem_init(void)
{
    sr_type sr;

    MTM_INTERRUPT_DISABLE();
    mtm_task_free   = 0;

#ifdef MTM_MUTEX_ENABLE
    mtm_mutex_free  = 0;
#endif

#ifdef MTM_SEM_ENABLE
    mtm_sem_free    = 0;
#endif

#ifdef MTM_MBOX_ENABLE
    mtm_mbox_free   = 0;
#endif

#ifdef MTM_MQUEUE_ENABLE
    mtm_mqueue_free = 0;
#endif
    MTM_INTERRUPT_ENABLE();
}

struct mtm_task_t *mtm_mem_get(void)
{
    sr_type sr;
    struct mtm_task_t *ptask;

    MTM_INTERRUPT_DISABLE();
    ptask = &mtm_task_table[mtm_task_free];
    mtm_task_free++;
    MTM_INTERRUPT_ENABLE();
    return ptask;
}

#ifdef MTM_MUTEX_ENABLE
struct mtm_mutex_t *mtm_mem_get_mutex(void)
{
    sr_type sr;
    struct mtm_mutex_t *pmutex;

    MTM_INTERRUPT_DISABLE();
    pmutex = &mtm_mutex_table[mtm_mutex_free];
    mtm_mutex_free++;
    MTM_INTERRUPT_ENABLE();
    return pmutex;
}
#endif

#ifdef MTM_SEM_ENABLE
struct mtm_sem_t *mtm_mem_get_sem(void)
{
    sr_type sr;
    struct mtm_sem_t *psem;

    MTM_INTERRUPT_DISABLE();
    psem = &mtm_sem_table[mtm_sem_free];
    mtm_sem_free++;
    MTM_INTERRUPT_ENABLE();
    return psem;
}
#endif

#ifdef MTM_MBOX_ENABLE
struct mtm_mbox_t *mtm_mem_get_mbox(void)
{
    sr_type sr;
    struct mtm_mbox_t *pmbox;

    MTM_INTERRUPT_DISABLE();
    pmbox = &mtm_mbox_table[mtm_mbox_free];
    mtm_mbox_free++;
    MTM_INTERRUPT_ENABLE();
    return pmbox;
}
#endif

#ifdef MTM_MQUEUE_ENABLE
struct mtm_mqueue_t *mtm_mem_get_mqueue(void)
{
    sr_type sr;
    struct mtm_mqueue_t *pmq;

    MTM_INTERRUPT_DISABLE();
    pmq = &mtm_mqueue_table[mtm_mqueue_free];
    mtm_mqueue_free++;
    MTM_INTERRUPT_ENABLE();
    return (pmq);
}
#endif

/* END OF FILE */
