#include <grrlib.h>
#include <wiiuse/wpad.h>
#include "input.h"
#include "packet.h"
#include "text.h"
#include "client.h"

int screen_width = 640;
int screen_height = 480;
GRRLIB_texImg* emulator_texture;

void InitializeMenu()
{
    GRRLIB_Init();
    GXRModeObj* vmode = VIDEO_GetPreferredMode(NULL);

	if(CONF_GetAspectRatio() == CONF_ASPECT_16_9)
		vmode->viWidth = VI_MAX_WIDTH_PAL;

    screen_width = vmode->viWidth;
    emulator_texture = GRRLIB_CreateEmptyTexture(screen_width, screen_height);

    InitializeText();
    InitializeInput();
    Printf("hydra-wii v0.1");
    InitializeClient();
}

void LoopMenu()
{
    while(1) {
        if (UpdateInput()) break;
        SendPacket(HC_PACKET_TYPE_video);
        GRRLIB_DrawImg(100, 0, emulator_texture, 0, 1, 1, 0xFFFFFFFF);
        DrawText();
        GRRLIB_Render();
    }
}

void DestroyMenu()
{
    GRRLIB_Exit();
}