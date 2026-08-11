# Basic data structures

## Ideal
All Just Header Files that allow for the implementation of the data Structures listed below with examples under examples/<data Type>

All implemenations are built to be threadsafe and atomic for use in any project.
# Case conventions
```c
//functions:
name_extra()
//variables
nameExtra
//structs
Name_Extra
//enums
name_extra_t 
//enum values
name_Extra_Example

```

# Stack LIFO

To begin using the stack it first needs to be intialised
``` c
<dataType> stackData[64];

Stack stack = {
    .data = stackData,
    .dataSize = sizeof(stackData),
    .pointerIndex = 0,
    .lock = ATOMIC_FLAG_INIT
};
```

## Stack Opperations
```c
stack_err_t push(Stack *stack, const void *const data, const size_t numberOfBytes);
stack_err_t pop(Stack *stack, void *const outData, const size_t numberOfBytes);
stack_err_t peek(Stack *stack, void *const outData, const size_t numberOfBytes);
```

All opperations take in a data pointer to write to/from and will always return an stack_err_t
When an opperation is being done to the stack. It is locked whilst the data is read/written. If the stack is already being pushed or poped the thread will spin until it gains the lock required to read/write.






# Queue FIFO


