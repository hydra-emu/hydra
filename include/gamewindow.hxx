#pragma once

#include "hydra/core.hxx"
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

    bool isLoaded()
    {
        return loaded;
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
    bool loaded = false;
    GLuint texture = 0;
    GLuint fbo = 0;
    ImVec2 start_drag;
    hydra::Size texture_size;
    hydra::PixelFormat pixel_format;
    GLenum gl_format;

    std::shared_ptr<hydra::EmulatorWrapper> emulator;

    static GameWindow* instance;
    static void video_callback(void* data, hydra::Size size);
    static GLenum get_gl_format(hydra::PixelFormat format);
};