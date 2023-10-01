#include <stdio.h>

void* memset(void *dst, int val, size_t count)
{
   void* start = dst;
   while (count--)
    *(char*)dst++ = (char)val;
   return start;
}