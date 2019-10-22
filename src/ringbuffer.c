/**
 * @file ringbuffer.c
 * @brief A simple ring buffer, can set fixed item size.
 * @author WANG Jun
 * @date 2017-05-22
 * 
 * update:
 * 2017-05-22   WANG Jun    init version.
 * 2017-06-21   WANG Jun    add unit testing macro.
 * 2017-08-22   WANG Jun    fixed pointer error.
 */

#include "frameworks.h"
#include <FreeRTOS.h>

#if UNIT_TESTING
#ifdef FW_ENABLE_MALLOC
extern void *_test_malloc(const size_t size, const char *file, const int line);
extern void _test_free(void *const ptr, const char *file, const int line);

#undef pvPortMalloc
#define pvPortMalloc(size) _test_malloc(size, __FILE__, __LINE__)
#undef vPortFree
#define vPortFree(ptr) _test_free(ptr, __FILE__, __LINE__)
#endif

extern void mock_assert(const int result, const char *const expression,
                        const char *const file, const int line);

#undef FW_ASSERT
#define FW_ASSERT(x) mock_assert((int)(x), #x, __FILE__, __LINE__)
#endif /* UNIT_TESTING */

int RingBufferInit(RingBuffer_t *rb, unsigned char *pool, int itemsize, int size)
{
    FW_ASSERT(rb != NULL);
    FW_ASSERT(pool != NULL);
    FW_ASSERT(itemsize > 0);
    FW_ASSERT(size > 0);

    rb->buf      = pool;
    rb->itemsize = itemsize;
    rb->size     = size; /* User should promise all memories have priorty to visit. */
    rb->head     = 0;
    rb->tail     = 0;
    memset(rb->buf, 0, rb->itemsize * rb->size);

    return (rb->itemsize * rb->size);
}

#ifdef FW_ENABLE_MALLOC
RingBuffer_t *RingBufferCreate(int itemsize, int size)
{
    RingBuffer_t *rb = NULL;

    FW_ASSERT(itemsize > 0);
    FW_ASSERT(size > 0);

    rb = pvPortMalloc(sizeof(RingBuffer_t));
    if (rb == NULL)
    {
        return NULL;
    }
    rb->itemsize = itemsize;
    rb->size     = FW_ALIGN_DOWN(itemsize * size, 4) / itemsize;
    rb->head     = 0;
    rb->tail     = 0;
    rb->buf      = pvPortMalloc(rb->itemsize * rb->size);
    if (rb->buf == NULL)
    {
        vPortFree(rb);
        return NULL;
    }
    memset(rb->buf, 0, rb->itemsize * rb->size);
    return rb;
}

void RingBufferDestroy(RingBuffer_t *rb)
{
    FW_ASSERT(rb != NULL);

    vPortFree(rb->buf);
    vPortFree(rb);
    /* After free(), rb pointer should be set to NULL manually. */
}
#endif

int RingBufferWrite(RingBuffer_t *rb, void *data, int length, int mode)
{
    int free_num = -1;

    FW_ASSERT(rb != NULL);
    FW_ASSERT(data != NULL);
    FW_ASSERT(length > 0);
    FW_ASSERT(IS_RINGBUFFER_WRITE_MODE(mode));

    if (length >= rb->size)
    {
        FWLOG_ERR("The length<%d> is too long to write in ring buffer.", length);
        return 0;
    }

    free_num = RingBufferQueryFree(rb);
    if (mode != RING_BUFFER_MODE_OVERWRITE)
    {
        if (free_num == 0)
            return 0;
        if (free_num < length)
            length = free_num;
    }

    if (rb->size - rb->head > length)
    {
        memcpy(&rb->buf[rb->itemsize * rb->head], data, rb->itemsize * length);
        rb->head += length;
    }
    else
    {
        memcpy(&rb->buf[rb->itemsize * rb->head], data, rb->itemsize * (rb->size - rb->head));
        memcpy(&rb->buf[0], data + (rb->itemsize * (rb->size - rb->head)),
               rb->itemsize * (length - (rb->size - rb->head)));
        rb->head = length - (rb->size - rb->head);
    }

    if ((mode == RING_BUFFER_MODE_OVERWRITE) && (free_num < length))
    {
        rb->tail = (rb->head + 1) % rb->size;
    }
    return length;
}

int RingBufferRead(RingBuffer_t *rb, void *data, int length, int mode)
{
    int data_num = -1;

    FW_ASSERT(rb != NULL);
    FW_ASSERT(data != NULL);
    FW_ASSERT(length > 0);
    FW_ASSERT(IS_RINGBUFFER_READ_MODE(mode));

    data_num = RingBufferQueryUsed(rb);
    if (data_num == 0)
        return 0;
    /* not enough data */
    if (data_num < length)
        length = data_num;

    if (rb->size - rb->tail > length)
    {
        memcpy(data, &rb->buf[rb->itemsize * rb->tail], rb->itemsize * length);
        if (mode != RING_BUFFER_MODE_READONLY)
            rb->tail += length;
        return length;
    }

    memcpy(data, &rb->buf[rb->itemsize * rb->tail], rb->itemsize * (rb->size - rb->tail));
    memcpy(data + (rb->itemsize * (rb->size - rb->tail)), &rb->buf[0],
           rb->itemsize * (length - (rb->size - rb->tail)));
    if (mode != RING_BUFFER_MODE_READONLY)
        rb->tail = length - (rb->size - rb->tail);
    return length;
}

/* vim:set et ts=4 sts=4 sw=4 ft=c: */
