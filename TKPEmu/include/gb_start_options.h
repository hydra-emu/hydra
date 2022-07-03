#pragma once
#ifndef TKP_GB_START_OPTIONS_H
#define TKP_GB_START_OPTIONS_H
#include <string>
#include <array>
struct GameboyOptions {
    std::array<uint32_t, 4> DMGColors;
    std::array<uint32_t, 4> DirectionMappings;
    std::array<uint32_t, 4> ActionMappings;
};
#endif