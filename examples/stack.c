#include "stack.h"
#include <stdio.h>

int main(void)
{
    static uint8_t stackData[64];

    Stack stack = {
        .data = stackData,
        .dataSize = sizeof(stackData),
        .pointerIndex = 0,
        .lock = ATOMIC_FLAG_INIT};

    char *g = "G";
    push(&stack, g, sizeof(char) * 2);
    int x = 2;
    push(&stack, &x, sizeof(int));
    x = 3;
    push(&stack, &x, sizeof(int));

    int y = 0;
    pop(&stack, &y, sizeof(int));
    printf("%d\n", y);
    pop(&stack, &y, sizeof(int));
    printf("%d\n", y);
    char s[2];
    pop(&stack, &s, sizeof(char) * 2);
    printf("%s\n", s);
    return 0;
}