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

//------------------------------------------------------------------------------
// Structures

/**
Usage:
<dataType> stackData[64];
Stack stack = {.data = stackData,.dataSize = sizeof(stackData)}
*/
typedef struct Stack
{
    volatile uint8_t *const data;
    const size_t dataSize;
    volatile size_t pointerIndex;
} Stack;

/*
All functions return this type to see what happened in the function
*/
typedef enum stack_err_t
{
    stack_Ok,
    stack_Full,
    stack_Insuffient_Space,
    stack_Empty,
} stack_err_t;

//------------------------------------------------------------------------------
// Function Definitions

/* Returns if stack is full via a stack_err_t */
static inline stack_err_t is_full(const Stack *const stack)
{
    if (stack->pointerIndex >= stack->dataSize)
    {
        return stack_Full;
    }
    return stack_Ok;
}

static inline size_t remaining_space(const Stack *const stack)
{
    return stack->dataSize - stack->pointerIndex;
}

static inline stack_err_t push(Stack *stack, const void *const data, const size_t numberOfBytes)
{
    stack_err_t err = is_full(stack);
    if (err != stack_Ok)
    {
        return err;
    }

    // check if sizeof(data) >= Remaining Size Left on stack
    if (remaining_space(stack) < numberOfBytes)
    {
        return stack_Insuffient_Space;
    }

    memcpy((void *)&stack->data[stack->pointerIndex],data,numberOfBytes);
    stack->pointerIndex += numberOfBytes;
    return stack_Ok;
}

static inline stack_err_t pop()
{
}

#endif