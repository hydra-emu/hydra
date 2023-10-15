#include <stdlib.h>
extern "C" {
#include "menu.h"
}

int main()
{
    InitializeMenu();
    LoopMenu();
    DestroyMenu();
    exit(0);
}