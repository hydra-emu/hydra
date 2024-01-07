#pragma once

#include "imgui.h"
#include <corewrapper.hxx>
#include <glad/glad.h>

enum class UpdateResult
{
    None,
    Quit,
};

class GameWindow
{
public:
    GameWindow(const std::string& core_path, const std::string& game_path);
    UpdateResult update();

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
    GLuint fbo = 0;
    ImVec2 start_drag;

    std::shared_ptr<hydra::EmulatorWrapper> emulator;
};