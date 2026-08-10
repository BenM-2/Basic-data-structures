#include "stack.h"

int main(void)
{
    static int stackData[128];
    Stack stack = {.data = stackData, .dataSize = sizeof(stackData)};
    char stack_bottom = "&";
    push(&stack,stack_bottom,sizeof(stack_bottom));
    return 0;
}