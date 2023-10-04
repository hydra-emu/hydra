#include <stdio.h>

void panic(const char* msg)
{
    printf("PANIC: %s\n", msg);
    while (1) {
        volatile const char* ptr = msg;
    };
}