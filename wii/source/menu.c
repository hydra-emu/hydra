#include <grrlib.h>
#include <wiiuse/wpad.h>
#include "input.h"
#include "client.h"

int screen_width = 640;
int screen_height = 480;

void InitializeMenu()
{
    GRRLIB_Init();
    GXRModeObj* vmode = VIDEO_GetPreferredMode(NULL);

	if(CONF_GetAspectRatio() == CONF_ASPECT_16_9)
		vmode->viWidth = VI_MAX_WIDTH_PAL;

    screen_width = vmode->viWidth;

    InitializeInput();
    InitializeClient();
}

void LoopMenu()
{
    while(1) {
        if (UpdateInput()) break;
        GRRLIB_Render();
    }
}

void DestroyMenu()
{
    GRRLIB_Exit();
}