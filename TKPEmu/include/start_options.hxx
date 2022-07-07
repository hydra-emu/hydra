#pragma once
#ifndef TKP_GB_START_OPTIONS_H
#define TKP_GB_START_OPTIONS_H
#include <string>
#include <array>
#include <filesystem>
#include <memory>
#include <fstream>
#include "emulator_types.hxx"
#include "error_factory.hxx"
#include "json.hpp"
using json = nlohmann::json;

struct EmulatorData {
    std::string Name;
    std::string SettingsFile;
    std::vector<std::string> Extensions;
    int DefaultWidth;
    int DefaultHeight;
    bool HasDebugger;
    bool HasTracelogger;
    std::vector<std::string> LoggingOptions;
};
struct OptionsBase {};
struct GameboyOptions : public OptionsBase {
    std::vector<uint32_t> DMGColors;
    std::vector<uint32_t> DirectionMappings;
    std::vector<uint32_t> ActionMappings;
};
#endif