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
    uint8_t *const data;
    size_t const dataSize;

    atomic_size_t writeIndex;
    atomic_size_t readIndex;

    atomic_flag writeLock;
} Mpsc_Queue;

typedef enum
{
    mpsc_Queue_Ok,
    mpsc_Queue_Insufficient_Space,
    mpsc_Queue_Insufficient_Data,
} mpsc_queue_err_t;

//------------------------------------------------------------------------------
// Inline Function Implementations

static inline void mpsc_queue_init(Mpsc_Queue *const mpsc_queue)
{
    atomic_store_explicit(&mpsc_queue->writeIndex, 0, memory_order_relaxed);
    atomic_store_explicit(&mpsc_queue->readIndex, 0, memory_order_relaxed);
    atomic_flag_clear_explicit(&mpsc_queue->writeLock, memory_order_relaxed);
}
 

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
    size_t readIndex = atomic_load_explicit(&mpsc_queue->readIndex, memory_order_acquire);
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
        memcpy((void *)&mpsc_queue->data[writeIndex], data, numberOfBytes);
        return writeIndex + numberOfBytes;
    }

    // before wrap
    size_t beforeWrap = mpsc_queue->dataSize - writeIndex;
    memcpy((void *)&mpsc_queue->data[writeIndex], data, beforeWrap);

    // After wrap
    size_t afterWrap = numberOfBytes - beforeWrap;
    memcpy(&mpsc_queue->data[0], (uint8_t *)data + beforeWrap, afterWrap);

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

    size_t writeIndex = atomic_load_explicit(&mpsc_queue->writeIndex, memory_order_relaxed);

    size_t newWriteIndex = mpsc_queue_write_internal(mpsc_queue, data, numberOfBytes, writeIndex);

    atomic_store_explicit(&mpsc_queue->writeIndex, newWriteIndex, memory_order_release);
    mpsc_queue_unlock(mpsc_queue);
    return mpsc_Queue_Ok;
}

static inline size_t mpsc_queue_read_internal(Mpsc_Queue *const mpsc_queue, void *const outData, size_t numberOfBytes)
{
    size_t readIndex = atomic_load_explicit(&mpsc_queue->readIndex, memory_order_relaxed);

    if (readIndex + numberOfBytes < mpsc_queue->dataSize)
    {
        memcpy(outData, (void *)&mpsc_queue->data[readIndex], numberOfBytes);
        return readIndex + numberOfBytes;
    }

    // before wrap
    size_t beforeWrap = mpsc_queue->dataSize - readIndex;
    memcpy(outData, (void *)&mpsc_queue->data[readIndex], beforeWrap);

    // After wrap
    size_t afterWrap = numberOfBytes - beforeWrap;
    memcpy((uint8_t *)outData + beforeWrap, &mpsc_queue->data[0], afterWrap);

    return afterWrap;
}

static inline mpsc_queue_err_t mpsc_queue_pop(Mpsc_Queue *const mpsc_queue, void *const outData, size_t numberOfBytes)
{
    size_t available_bytes = mpsc_queue_available_read(mpsc_queue);
    if (available_bytes < numberOfBytes)
    {
        return mpsc_Queue_Insufficient_Data;
    }

    
    size_t newReadIndex = mpsc_queue_read_internal(mpsc_queue, outData, numberOfBytes);
    atomic_store_explicit(&mpsc_queue->readIndex, newReadIndex, memory_order_release);
    return mpsc_Queue_Ok;
}

static inline mpsc_queue_err_t mpsc_queue_peek(Mpsc_Queue *const mpsc_queue, void *const outData, size_t numberOfBytes)
{
    size_t available_bytes = mpsc_queue_available_read(mpsc_queue);
    if (available_bytes < numberOfBytes)
    {
        return mpsc_Queue_Insufficient_Data;
    }

    
    mpsc_queue_read_internal(mpsc_queue, outData, numberOfBytes);
    return mpsc_Queue_Ok;
}

static inline mpsc_queue_err_t mpsc_queue_pop_all(Mpsc_Queue *const mpsc_queue, void *const outData)
{
    size_t available_bytes = mpsc_queue_available_read(mpsc_queue);    
    size_t newReadIndex = mpsc_queue_read_internal(mpsc_queue, outData, available_bytes);
    atomic_store_explicit(&mpsc_queue->readIndex, newReadIndex, memory_order_release);
    return mpsc_Queue_Ok;
}
#endif