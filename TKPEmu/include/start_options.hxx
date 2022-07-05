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
constexpr const char* GameboyDefault = R"json({
    "DMGColors": [
        16777215,
        12632256,
        8421504,
        0
    ],
    "DirectionMappings": [
        16777236,
        16777234,
        16777235,
        16777237
    ],
    "ActionMappings": [
        90,
        88,
        32,
        16777220
    ]
})json";

struct OptionsBase {};
struct GameboyOptions : public OptionsBase {
    std::vector<uint32_t> DMGColors;
    std::vector<uint32_t> DirectionMappings;
    std::vector<uint32_t> ActionMappings;
};
static std::string GetSavePath() {
    static std::string dir;
    if (dir.empty()) {
        #if defined(__linux__)
        dir = getenv("HOME") + std::string("/.config/tkpemu/");
        #elif defined(_WIN32)
        dir = getenv("APPDATA") + std::string("/tkpemu/");
        #endif
        if (dir.empty()) {
            throw ErrorFactory::generate_exception(__func__, __LINE__, "GetSavePath was not defined for this environment");
        }
        if (!std::filesystem::create_directories(dir)) {
            if (std::filesystem::exists(dir))
                return dir;
            throw ErrorFactory::generate_exception(__func__, __LINE__, "Failed to create directories");
        }
    }
    return dir;
}
static std::unique_ptr<OptionsBase> GetOptions(TKPEmu::EmuType type) {
    using nlohmann::json;
    switch (type) {
        case TKPEmu::EmuType::Gameboy: {
            auto path = GetSavePath() + "gameboy.json";
            std::unique_ptr<OptionsBase> ret(new GameboyOptions);
            auto gb = static_cast<GameboyOptions*>(ret.get());
            if (std::filesystem::exists(path)) {
                std::ifstream ifs(path);
                if (ifs.is_open()) {
                    json j;
                    ifs >> j;
                    j.at("DMGColors").get_to(gb->DMGColors);
                    j.at("DirectionMappings").get_to(gb->DirectionMappings);
                    j.at("ActionMappings").get_to(gb->ActionMappings);
                    ifs.close();
                }
            } else {
                std::ofstream ofs(path);
                if (ofs.is_open()) {
                    ofs << GameboyDefault;
                    ofs.close();
                    std::ifstream ifs(path);
                    if (ifs.is_open()) {
                        json j;
                        ifs >> j;
                        j.at("DMGColors").get_to(gb->DMGColors);
                        j.at("DirectionMappings").get_to(gb->DirectionMappings);
                        j.at("ActionMappings").get_to(gb->ActionMappings);
                        ifs.close();
                    }
                } else {
                    throw ErrorFactory::generate_exception(__func__, __LINE__, "Could not create gameboy settings file");
                }
            }
            return ret;
            break;
        }
        default:
        return nullptr;
    }
}
#endif