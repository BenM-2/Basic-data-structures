#include "mpsc_queue.h"
#include "stdio.h"
#include <stdlib.h>

int main(int argc, char *argv[])
{
    static uint8_t data[120];
    Mpsc_Queue q = {
        .data = data,
        .dataSize = sizeof(data),
    };
    mpsc_queue_init(&q);

    int x = atoi(argv[1]);

    mpsc_queue_push(&q, &x, sizeof(int));

    int y;
    mpsc_queue_pop(&q,&y,sizeof(int));
    printf("%d\n",y);
    return 0;
}