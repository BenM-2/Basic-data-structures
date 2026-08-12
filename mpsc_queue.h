/**
 * @file mpsc_queue.h
 * @author Ben Marples
 */

#ifndef MPSC_QUEUE_IMPL
#define MPSC_QUEUE_IMPL

//------------------------------------------------------------------------------
// includes
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <stdatomic.h>

//------------------------------------------------------------------------------
// Structures

typedef struct Mpsc_Queue
{
    volatile uint8_t *const data;
    size_t const dataSize;

    atomic_size_t writeIndex;
    size_t readIndex;

    atomic_flag writeLock;
} Mpsc_Queue;

typedef enum
{
    mpsc_Queue_Ok,
    mpsc_Queue_Insufficient_Space,
} mpsc_queue_err_t;

//------------------------------------------------------------------------------
// Inline Function Implementations

static inline void mpsc_queue_lock(Mpsc_Queue *const mpsc_queue)
{
    while (atomic_flag_test_and_set_explicit(&mpsc_queue->writeLock, memory_order_acquire))
    {
        // spin
    }
}

static inline void mpsc_queue_unlock(Mpsc_Queue *const mpsc_queue)
{
    atomic_flag_clear_explicit(&mpsc_queue->writeLock, memory_order_release);
}

static inline size_t mpsc_queue_available_read(const Mpsc_Queue *const mpsc_queue)
{
    size_t readIndex = mpsc_queue->readIndex;
    size_t writeIndex = atomic_load_explicit(&mpsc_queue->writeIndex, memory_order_acquire);

    if (writeIndex < readIndex)
    {
        return writeIndex + (mpsc_queue->dataSize - readIndex);
    }
    return writeIndex - readIndex;
}

static inline size_t mpsc_queue_available_write(const Mpsc_Queue *const mpsc_queue)
{
    // Available write = Total - Written(readable)
    size_t available_write = (mpsc_queue->dataSize - 1) - mpsc_queue_available_read(mpsc_queue);
    return available_write;
}

static inline size_t mpsc_queue_write_internal(Mpsc_Queue *const mpsc_queue, const void *const data, size_t numberOfBytes, size_t writeIndex)
{
    if ((writeIndex + numberOfBytes) < mpsc_queue->dataSize)
    {
        memcpy(mpsc_queue->data[writeIndex], data, numberOfBytes);
        return writeIndex + numberOfBytes;
    }

    // before wrap
    size_t beforeWrap = mpsc_queue->dataSize - writeIndex;
    memcpy(mpsc_queue->data[writeIndex], data, beforeWrap);

    // After wrap
    size_t afterWrap = numberOfBytes - beforeWrap;
    memcpy(mpsc_queue->data, (uint8_t *)data + beforeWrap, afterWrap);

    return afterWrap;
}

static inline mpsc_queue_err_t mpsc_queue_push(Mpsc_Queue *const mpsc_queue, const void *const data, size_t numberOfBytes)
{
    mpsc_queue_lock(mpsc_queue);
    size_t available_bytes = mpsc_queue_available_write(mpsc_queue);
    if (available_bytes < numberOfBytes)
    {
        mpsc_queue_unlock(mpsc_queue);
        return mpsc_Queue_Insufficient_Space;
    }

    size_t writeIndex = atomic_load_explicit(&mpsc_queue->writeIndex,memory_order_acquire);

    size_t newWriteIndex = mpsc_queue_write_internal(mpsc_queue,data,numberOfBytes,writeIndex);

    atomic_store_explicit(&mpsc_queue->writeIndex, newWriteIndex, memory_order_release);
    mpsc_queue_unlock(mpsc_queue);
    return mpsc_Queue_Ok;
}
#endif