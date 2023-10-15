#include "text.h"
#include "font_png.h"
#include <stdarg.h>
#include <grrlib.h>
#include <stdio.h>

int console_x = 25, console_y = 25;
GRRLIB_texImg* font = 0;

char** buffer;
int buffer_index = 0;

void InitializeText()
{
    font = GRRLIB_LoadTexture(font_png);
    GRRLIB_InitTileSet(font, 8, 16, 0);

    buffer = (char**)malloc(100 * sizeof(char*));
    memset(buffer, 0, 100 * sizeof(char*));
}

void Printf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int size = 256;
    char* msg = (char*)malloc(size * sizeof(char));
    memset(msg, 0, size * sizeof(char));
    vsnprintf(msg, size, format, args);
    buffer[buffer_index++] = msg;
    va_end(args);
}

void DrawText()
{
    console_x = 25;
    console_y = 25;

    for (int i = 0; i < 100; i++) {
        if (buffer[i] == 0) break;
        GRRLIB_Printf(console_x, console_y, font, 0xFFFFFFFF, 1, buffer[i]);
        console_y += 16;
    }
}