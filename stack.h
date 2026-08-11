/**
 * @file stack.h
 * @author Ben Marples
 */

#ifndef STACK_IMPL
#define STACK_IMPL

//------------------------------------------------------------------------------
// includes
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <stdatomic.h>

//------------------------------------------------------------------------------
// Structures

/**
Usage:
<dataType> stackData[64];
Stack stack = {.data = stackData,.dataSize = sizeof(stackData), .lock=ATOMIC_FLAG_INIT}
*/
typedef struct Stack
{
    volatile uint8_t *const data;
    const size_t dataSize;
    volatile size_t pointerIndex;
    atomic_flag lock;
} Stack;

/*
All functions return this type to see what happened in the function
*/
typedef enum stack_err_t
{
    stack_Ok,
    stack_Full,
    stack_Insufficient_Space,
    stack_Invalid_Read_Size,
    stack_Empty,
    stack_Locked,
} stack_err_t;

//------------------------------------------------------------------------------
// Function Definitions

/* Returns if stack is full via a stack_err_t */
static inline void stack_lock(Stack *const stack)
{
    while (atomic_flag_test_and_set_explicit(&stack->lock, memory_order_acquire))
    {
        // spin: another context holds the lock
    }
}

static inline void stack_unlock(Stack *const stack)
{
    atomic_flag_clear_explicit(&stack->lock, memory_order_release);
}

static inline stack_err_t is_full(const Stack *const stack)
{
    if (stack->pointerIndex >= stack->dataSize)
    {
        return stack_Full;
    }
    return stack_Ok;
}

static inline stack_err_t is_empty(const Stack *const stack)
{
    if (stack->pointerIndex == 0)
    {
        return stack_Empty;
    }
    return stack_Ok;
}

static inline size_t remaining_space(const Stack *const stack)
{
    return stack->dataSize - stack->pointerIndex;
}

static inline stack_err_t push(Stack *stack, const void *const data, const size_t numberOfBytes)
{
    stack_lock(stack);

    stack_err_t err = is_full(stack);
    if (err != stack_Ok)
    {
        stack_unlock(stack);
        return err;
    }

    // check if sizeof(data) >= Remaining Size Left on stack
    if (remaining_space(stack) < numberOfBytes)
    {
        stack_unlock(stack);
        return stack_Insufficient_Space;
    }

    memcpy((void *)&stack->data[stack->pointerIndex], data, numberOfBytes);
    stack->pointerIndex += numberOfBytes;
    stack_unlock(stack);
    return stack_Ok;
}

static inline stack_err_t pop(Stack *stack, void *const outData, const size_t numberOfBytes)
{
    stack_lock(stack);

    if (is_empty(stack) == stack_Empty)
    {
        stack_unlock(stack);
        return stack_Empty;
    }

    if (stack->pointerIndex < numberOfBytes)
    {
        stack_unlock(stack);
        return stack_Invalid_Read_Size;
    }

    const size_t newPtrIdx = stack->pointerIndex - numberOfBytes;
    memcpy(outData, (const void *)&stack->data[newPtrIdx], numberOfBytes);
    stack->pointerIndex = newPtrIdx;
    stack_unlock(stack);
    return stack_Ok;
}

static inline stack_err_t peek(Stack *stack, void *const outData, const size_t numberOfBytes)
{
    stack_lock(stack);

    if (is_empty(stack) == stack_Empty)
    {
        stack_unlock(stack);
        return stack_Empty;
    }

    if (stack->pointerIndex < numberOfBytes)
    {
        stack_unlock(stack);
        return stack_Invalid_Read_Size;
    }

    const size_t newPtrIdx = stack->pointerIndex - numberOfBytes;
    memcpy(outData, (const void *)&stack->data[newPtrIdx], numberOfBytes);
    stack_unlock(stack);
    return stack_Ok;
}

#endif