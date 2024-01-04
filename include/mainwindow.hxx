#pragma once

#include "imgui.h"
#include <array>
#include <string>

struct Star
{
    float x, y;
    float vel_x, vel_y;
};

struct MainWindow
{
    MainWindow();
    void update();

private:
    void draw_cores();
    void draw_about();
    void draw_settings();
    void draw_stars(ImVec2 center, float radius);

    float selected_y = 0.0f;
    int selected_tab = 0;
    int biggest_tab_size = 0;
    std::string core_directory;
    std::array<Star, 50> stars;
    bool fancy_gui;
};