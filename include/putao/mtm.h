
#ifndef __MTM_H__
#define __MTM_H__

#include "mtm_def.h"
#include "mtm_arch.h"
#include "mtm_conf.h"
#include "mtm_list.h"

/**
 * Macro Defines
 */
/* mtm defines */
#define MTM_USED
#define MTM_VERSION                 1u          /* mtm version */

/* Priority defines */
#define MTM_LOWEST_PRIO             7u          /* The lowest priority level         */
#define MTM_HIGHEST_PRIO            0u          /* The highest priority level        */

/* Type of event */
#define MTM_EVENT_TYPE_NONE         0u
#define MTM_EVENT_TYPE_SEMAPHORE    1u
#define MTM_EVENT_TYPE_MUTEX        2u

/* Task stat */
#define MTM_STAT_DORMANT            0u          /* Initial stat, or has been del     */
#define MTM_STAT_READY              2u          /* Task is ready                     */
#define MTM_STAT_SUSPEND            3u          /* Task is suspend if delay or other */

/* Pend type */
#define MTM_PENDTYPE_NONE           0x00
#define MTM_PENDTYPE_DELAY          0x10
#define MTM_PENDTYPE_MUTEX          0x20
#define MTM_PENDTYPE_SEM            0x30
#define MTM_PENDTYPE_MBOX           0x40
#define MTM_PENDTYPE_MQUEUE         0x50

/* Pend stat */
#define MTM_PENDSTAT_OK             0x00
#define MTM_PENDSTAT_TIMEOUT        0x01
#define MTM_PENDSTAT_ABORT          0x02


/* Timer stat */
#define MTM_TMRSTAT_NOT_USED        0x00
#define MTM_TMRSTAT_STOPPED         0x01
#define MTM_TMRSTAT_ABORT           0x02
#define MTM_TMRSTAT_RUNNING         0x03

/* Error code */
#define MTM_ERR_NONE                0u
#define MTM_ERR_ERROR               1u
#define MTM_ERR_PENDTYPE            2u
#define MTM_ERR_TIMEOUT             3u
#define MTM_ERR_NOT_MUTEX_OWNER     4u
#define MTM_ERR_MQUEUE_FULL         5u
#define MTM_ERR_MQUEUE_EMPTY        6u
#define MTM_ERR_MBOX_FULL           7u



/* Timer callback function type define */
typedef void    (*mtm_timer_callback)(void *callback_param);

/**
 * @brief Timer struct
 */
struct mtm_timer_t {
    struct mtm_list_t   list;                   /* Timer list                         */
    mtm_timer_callback  callback;               /* Timer callback function            */
    void                *cb_param;              /* Parameter of the callback function */
    tick_t              count;
    tick_t              compare;
    uint8_t             stat;
};


#ifdef MTM_MUTEX_ENABLE

/**
 * @brief Mutex struct
 */
struct mtm_mutex_t {
    struct mtm_list_t   list;
    uint8_t             value;
    uint8_t             hold_prio;
    struct mtm_task_t   *owner;
};

#endif

#ifdef MTM_SEM_ENABLE

/**
 * @brief Semaphore
 */
struct mtm_sem_t {
    struct mtm_list_t   list;
    uint16_t            value;
};

#define MTM_SEM_VALUE_MAX   UINT16_MAX

#endif

#ifdef MTM_MBOX_ENABLE

/**
 * @brief Mailbox
 */
struct mtm_mbox_t {
    struct mtm_list_t   mb_list;
    void                *mb_msg;
};

#endif

#ifdef MTM_MQUEUE_ENABLE

/**
 * @brief Message queue struct
 */
struct mtm_mqueue_t {
    struct mtm_list_t   list;
    void                **head;
    void                **tail;
    void                **in;
    void                **out;
    uint16_t            msg_nbr;                 /* Total number of message        */
    uint16_t            entries;                 /* Number of entries in queue     */
};

#endif

/**
 * @brief Task Control Block definitions
 */
struct mtm_task_t {
    stack_t             *stk;                    /* Stack top pointer              */
    struct mtm_list_t   list;                    /* Task list                      */
    uint8_t             orig_prio;
    uint8_t             prio;                    /* Task priority                  */
    uint8_t             stat;                    /* Task status                    */
    uint8_t             pend_type;
    uint8_t             pend_stat;               /* Suspend task status            */
    uint16_t            orig_tick;               /* Original ticks for polling     */
    uint16_t            tick;                    /* Ticks for task                 */
    struct mtm_timer_t  timer;
};


/**
 * @brief Global variables
 */
extern volatile uint8_t     mtm_sched_nesting;  /* Schedule nesting lock           */
extern volatile uint8_t     mtm_inter_nesting;  /* Interrupt nesting lock          */

extern volatile uint8_t     mtm_cur_prio;       /* Current running priority        */
extern volatile uint8_t     mtm_ready_prio;     /* Ready priorities                */
extern volatile uint8_t     mtm_highest_prio;   /* Current highest ready priority  */

extern struct mtm_list_t    mtm_ready_list[MTM_LOWEST_PRIO + 1]; /* Ready list of each prio */
extern struct mtm_task_t    *mtm_cur_task;      /* Current running task            */
extern struct mtm_task_t    *mtm_next_task;     /* Next task                       */


/**
 * @brief Function prototypes
 */
void mtm_init(void);
void mtm_start(void);

void     mtm_time_init(void);
uint32_t mtm_time_elapse(void);
void     mtm_sys_tick(void);

void mtm_int_enter(void);
void mtm_int_exit(void);
void mtm_sched_lock(void);
void mtm_sched_unlock(void);

void mtm_sched(void);
void mtm_sched_init(void);
void mtm_get_highest_prio(void);
void mtm_add_to_readylist(struct mtm_task_t *ptask);
void mtm_add_to_readylist_head(struct mtm_task_t *ptask);
void mtm_remove_from_readylist(struct mtm_task_t *ptask);


struct mtm_task_t * mtm_task_create(void              (*task_entry)(void *param),
                                    void              *param,
                                    stack_t           *stk_addr,
                                    uint32_t          stk_size,
                                    uint8_t           prio,
                                    uint16_t          tick);
void mtm_task_return(void);
void mtm_task_start(struct mtm_task_t *ptask);
void mtm_task_destroy(struct mtm_task_t *ptask);
void mtm_task_yield(void);
void mtm_task_suspend(struct mtm_task_t *ptask);
void mtm_task_resume(struct mtm_task_t *ptask);
void mtm_task_resume_head(struct mtm_task_t *ptask);
void mtm_task_change_prio(struct mtm_task_t *ptask, uint8_t prio);

void    mtm_task_delay(uint32_t tick);
uint8_t mtm_task_delay_resume(struct mtm_task_t *ptask);

void mtm_idle_task_init(void);
void mtm_idle_task(void *param);

void    mtm_timer_init(void);
uint8_t mtm_timer_create(struct mtm_timer_t *ptimer, uint32_t cnt, mtm_timer_callback callback, void *param);
uint8_t mtm_timer_destroy(struct mtm_timer_t *ptimer);
uint8_t mtm_timer_start(struct mtm_timer_t *ptimer);
uint8_t mtm_timer_stop(struct mtm_timer_t *ptimer);
uint8_t mtm_timer_manage(void);

void mtm_mem_copy(uint8_t *dst, const uint8_t *src, uint16_t size);
void   mtm_mem_init(void);
struct mtm_task_t   *mtm_mem_get(void);

#ifdef MTM_MUTEX_ENABLE
struct mtm_mutex_t  *mtm_mem_get_mutex(void);
#endif

#ifdef MTM_SEM_ENABLE
struct mtm_sem_t    *mtm_mem_get_sem(void);
#endif

#ifdef MTM_MBOX_ENABLE
struct mtm_mbox_t   *mtm_mem_get_mbox(void);
#endif

#ifdef MTM_MQUEUE_ENABLE
struct mtm_mqueue_t *mtm_mem_get_mqueue(void);
#endif

#ifdef MTM_MUTEX_ENABLE
uint8_t mtm_mutex_create(struct mtm_mutex_t *pmutex);
void    mtm_mutex_delete(void);
uint8_t mtm_mutex_signal(struct mtm_mutex_t *pmutex);
uint8_t mtm_mutex_wait(struct mtm_mutex_t *pmutex, uint32_t timeout);
uint8_t mtm_mutex_try(struct mtm_mutex_t *pmutex);
#endif

#ifdef MTM_SEM_ENABLE
uint8_t mtm_sem_create(struct mtm_sem_t *psem, uint16_t value);
void    mtm_sem_delete(void);
uint8_t mtm_sem_wait(struct mtm_sem_t *psem, uint16_t timeout);
uint8_t mtm_sem_signal(struct mtm_sem_t *psem);
uint8_t mtm_sem_try(struct mtm_sem_t *psem);
#endif

#ifdef MTM_MBOX_ENABLE
uint8_t mtm_mbox_create(struct mtm_mbox_t *pmbox, void *pmsg);
void    mtm_mbox_delete(void);
uint8_t mtm_mbox_send(struct mtm_mbox_t *pmbox, void *pmsg);
uint8_t mtm_mbox_receive(struct mtm_mbox_t *pmbox, void *pmsg, uint16_t timeout);
uint8_t mtm_mbox_try(struct mtm_mbox_t *pmbox, void *pmsg);
#endif

#ifdef MTM_MQUEUE_ENABLE
uint8_t mtm_mqueue_create(struct mtm_mqueue_t *pmq, void **msg_start, uint16_t msg_nbr);
void    mtm_mqueue_delete(void);
uint8_t mtm_mqueue_send(struct mtm_mqueue_t *pmq, void *pmsg);
uint8_t mtm_mqueue_receive(struct mtm_mqueue_t *pmq, void *pmsg, uint16_t timeout);
uint8_t mtm_mqueue_try(struct mtm_mqueue_t *pmq, void *pmsg);
#endif

/* mtm version */
mtm_inline uint8_t mtm_get_version(void)
{
    return (MTM_VERSION);
}

#endif /* __MTM_H__ */

/* END OF FILE */
