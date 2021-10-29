#pragma once
#ifndef TKP_APPLICATION_SETTINGS_H
#define TKP_APPLICATION_SETTINGS_H
#include <cstdint>
using AppSettingsType = uint8_t;
struct AppSettings {
    AppSettingsType limit_fps = 1;
    AppSettingsType max_fps_index = 1;
    AppSettingsType dmg_color0_r = 255;
    AppSettingsType dmg_color0_g = 208;
    AppSettingsType dmg_color0_b = 164;
    AppSettingsType dmg_color1_r = 244;
    AppSettingsType dmg_color1_g = 148;
    AppSettingsType dmg_color1_b = 156;
    AppSettingsType dmg_color2_r = 124;
    AppSettingsType dmg_color2_g = 154;
    AppSettingsType dmg_color2_b = 172;
    AppSettingsType dmg_color3_r = 104;
    AppSettingsType dmg_color3_g = 81;
    AppSettingsType dmg_color3_b = 138;
    // TODO: implement window size and fullscreen and widgets
    AppSettingsType window_size_index = 0;
    AppSettingsType window_fullscreen = 0;
    AppSettingsType debug_mode = 1;
};   
constexpr auto AppSettingsSize = sizeof(AppSettings) / sizeof(AppSettingsType);
#endif