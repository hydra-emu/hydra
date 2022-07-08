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

struct KeyMappings {
    std::vector<std::string> KeyNames;
    std::vector<uint32_t> KeyValues;
};
struct EmulatorData {
    std::string Name;
    std::string SettingsFile;
    std::vector<std::string> Extensions;
    int DefaultWidth;
    int DefaultHeight;
    bool HasDebugger;
    bool HasTracelogger;
    std::vector<std::string> LoggingOptions;
    KeyMappings Mappings;
};
struct OptionsBase {
    KeyMappings Mappings;
};
struct GameboyOptions : public OptionsBase {
    std::vector<uint32_t> DMGColors;
};
#endif