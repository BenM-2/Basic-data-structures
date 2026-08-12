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

#endif