#pragma once

#include "imgui.h"
#include <corewrapper.hxx>
#include <SDL3/SDL_opengl.h>

class GameWindow
{
public:
    GameWindow();
    void update();

    bool isFullscreen()
    {
        return fullscreen && !animating;
    }

private:
    float velocity_x = 0.0f;
    float velocity_y = 0.0f;
    float size_x = 0.0f;
    float size_y = 0.0f;
    float position_x = 0.0f;
    float position_y = 0.0f;
    int horizontal_snap = -1;
    int vertical_snap = -1;
    float snap_x = 0.0f;
    float snap_y = 0.0f;
    bool held = false;
    bool fullscreen = false;
    bool animating = false;
    GLuint texture = 0;
    ImVec2 start_drag;

    std::unique_ptr<hydra::EmulatorWrapper> emulator;
};