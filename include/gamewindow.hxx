#pragma once

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
    float snap_x = 0.0f;
    float snap_y = 0.0f;
    bool snapped = false;
    bool held = false;
    bool fullscreen = false;
    bool animating = false;
};