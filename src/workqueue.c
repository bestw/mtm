/**
 * @file workqueue.c
 * @brief 
 * @author WANG Jun
 * @version v1.00
 * @date: 2017-07-11
 * 
 * history:
 * 2017-07-11   WANG Jun  v1.0.0  init version.
 * 2017-11-29   WANG Jun    add WorkQueueCreate()
 */
#include "FreeRTOS.h"
#include "task.h"
#include "frameworks.h"

/**
 * @addtogroup WorkQueue
 * @{
 */

static void WorkQueueTaskEntry(void *param)
{
    Work_t *work       = NULL;
    WorkQueue_t *queue = (WorkQueue_t *)param;

    FW_ASSERT(queue != NULL);
    while (1)
    {
        taskENTER_CRITICAL();
        while (!ListIsEmpty(&queue->list))
        {
            work = LIST_NODE_ENTRY(queue->list.next, struct WorkStruct, node);
            ListDelete(&work->node);

            taskEXIT_CRITICAL();
            work->work_func(work, work->data);
            taskENTER_CRITICAL();
        }
        taskEXIT_CRITICAL();

        vTaskSuspend(queue->task);
    }
}

void DoWork(WorkQueue_t *queue, Work_t *work)
{
    struct list_head *plist = NULL;

    taskENTER_CRITICAL();
    LIST_FOR_EACH(plist, &queue->list)
    {
        if (plist == &work->node)
        {
            break;
        }
    }
    if (plist != &work->node)
    {
        ListAddTail(&work->node, &queue->list);
    }
    taskEXIT_CRITICAL();

    vTaskResume(queue->task);
}

#ifdef FW_ENABLE_MALLOC
WorkQueue_t *WorkQueueCreate(char *name, int stack_size, int task_prio)
{
    TaskHandle_t task;
    BaseType_t state;
    WorkQueue_t *new_work_queue;

    new_work_queue = pvPortMalloc(sizeof(WorkQueue_t));
    if (new_work_queue == NULL)
    {
        FWLOG_WARNING("Can not malloc work queue.");
        return NULL;
    }

    state = xTaskCreate(WorkQueueTaskEntry, name,
                        stack_size, new_work_queue, task_prio, &task);
    if (state != pdPASS)
    {
        FWLOG_WARNING("Can not create work queue task.");
        return NULL;
    }
    ListInit(&new_work_queue->list);
    new_work_queue->task       = task;
    new_work_queue->task_prio  = task_prio;
    new_work_queue->stack_size = stack_size;
    new_work_queue->name       = name;

    return new_work_queue;
}

Work_t *WorkCreate(char *name, WorkFunction_t work_func, void *data)
{
    Work_t *new_work;

    new_work = pvPortMalloc(sizeof(Work_t));
    if (new_work != NULL)
    {
        FWLOG_WARNING("Can not malloc work data.");
        return NULL;
    }

    ListInit(&new_work->node);
    new_work->work_name = name;
    new_work->work_func = work_func;
    new_work->data      = data;

    return new_work;
}
#endif
/** @} */

/* vim:set et ts=4 sts=4 sw=4 ft=c: */
