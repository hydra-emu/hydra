#pragma once

#include "filepicker.hxx"
#include "gamewindow.hxx"
#include "settings.hxx"
#include <array>
#include <bot.hxx>
#include <deque>
#include <filesystem>
#include <functional>
#include <imgui/imgui.h>
#include <string>
#include <unordered_map>
#include <vector>

struct Star
{
    float x, y;
    float vel_x, vel_y;
};

struct MainWindow
{
    MainWindow();
    void update();
    void loadRom(const std::filesystem::path& path);

    void setSettingsFunctions(
        const std::unordered_map<std::string, std::vector<std::function<void()>>>& functions)
    {
        settings_functions = functions;
    }

private:
    void draw_games();
    void draw_cores();
    void draw_input();
    void draw_about();
    void draw_bot();
    void draw_settings();
    void draw_stars(ImVec2 center, float radius);
    void draw_pending_rom_load();
    std::tuple<bool, ImVec2, ImVec2> draw_custom_button(const std::string& text,
                                                        uint32_t color = 0x40404040,
                                                        uint32_t hover_color = 0x80808080);
    void update_impl();
    void load_rom_impl(const std::filesystem::path& core_path,
                       const std::filesystem::path& rom_path);
    void save_recents();

    float selected_y = 0.0f;
    int selected_tab = 0;
    int biggest_tab_size = 0;
    std::string core_directory;
    std::array<Star, 50> stars;
    bool fancy_gui;
    size_t open_core_settings = -1;
    std::unordered_map<std::string, std::vector<std::function<void()>>> settings_functions;
    std::unique_ptr<GameWindow> game_window;
    std::deque<std::filesystem::path> recent_roms;
    std::string bot_token;

    bool pending_rom_load = false;
    std::filesystem::path pending_rom_path;
    std::vector<hydra::CoreInfo> pending_available_cores;
    std::vector<std::string> filters;

    FilePicker rom_picker;
};