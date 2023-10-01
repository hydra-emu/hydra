#include <grrlib.h>
#include <wiiuse/wpad.h>

void InitializeMenu()
{
    GRRLIB_Init();
    WPAD_Init();
}

void LoopMenu()
{
    while(1) {
        WPAD_ScanPads();
        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME)  break;
        GRRLIB_Render();
    }
}

void DestroyMenu()
{
    GRRLIB_Exit();
}