#pragma once

struct MainWindow
{
    void update();

private:
    void draw_cores();
    void draw_about();
    void draw_settings();

    int selected_tab = 0;
};