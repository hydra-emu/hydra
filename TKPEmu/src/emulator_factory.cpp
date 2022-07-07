#include <iostream>
#include <lib/str_hash.h>
#include <include/emulator_factory.h>
#include <include/error_factory.hxx>
#include <include/start_options.hxx>

namespace TKPEmu {
    EmulatorDataMap EmulatorFactory::emulator_data_{};
    ExtensionMappings EmulatorFactory::extension_mappings_{};
    std::string EmulatorFactory::GetSavePath() {
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
    std::unique_ptr<OptionsBase> EmulatorFactory::GetOptions(TKPEmu::EmuType type) {
        using nlohmann::json;
        auto path = GetSavePath() + emulator_data_[static_cast<int>(type)].SettingsFile;
        switch (type) {
            case TKPEmu::EmuType::Gameboy: {
                std::unique_ptr<OptionsBase> ret(new GameboyOptions);
                auto gb = static_cast<GameboyOptions*>(ret.get());
                std::ifstream ifs(path);
                if (ifs.is_open()) {
                    json j;
                    ifs >> j;
                    j.at("DMGColors").get_to(gb->DMGColors);
                    j.at("DirectionMappings").get_to(gb->DirectionMappings);
                    j.at("ActionMappings").get_to(gb->ActionMappings);
                    ifs.close();
                } else {
                    throw ErrorFactory::generate_exception(__func__, __LINE__, std::string("File does not exist:") + path);
                }
                return ret;
                break;
            }
            default:
            return nullptr;
        }
    }
    // TODO: detect emutype by magic bytes instead of extension
    EmuType EmulatorFactory::GetEmulatorType(std::filesystem::path path) {
        auto ext = path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(),
            [](unsigned char c){ return std::tolower(c); });
        try {
            return extension_mappings_.at(ext);
        } catch (...) {
            throw ErrorFactory::generate_exception(__func__, __LINE__, "Unknown file extension");
        }
    }
    std::shared_ptr<Emulator> EmulatorFactory::Create(EmuType type) { 
        auto args = GetOptions(type);
        switch (type) {
            case EmuType::Gameboy: {
                return std::make_shared<Gameboy::Gameboy>(std::move(args));
            }
            case EmuType::N64: {
                return std::make_shared<N64::N64_TKPWrapper>(std::move(args));
            }
            case EmuType::Chip8: {
                return std::make_shared<Chip8::Chip8>(std::move(args));
            }
            case EmuType::NES: {
                return std::make_shared<NES::NES>(std::move(args));
            }
            default: {
                throw ErrorFactory::generate_exception(__func__, __LINE__, "EmulatorFactory::Create failed");
            }
        }
    }
    void EmulatorFactory::SetEmulatorData(EmulatorDataMap map) {
        EmulatorFactory::emulator_data_ = std::move(map);
        // Map extensions to EmuType
        for (int i = 0; i < EmulatorFactory::emulator_data_.size(); i++) {
            auto& data = EmulatorFactory::emulator_data_[i];
            for (auto& ext : data.Extensions) {
                EmulatorFactory::extension_mappings_[ext] = static_cast<EmuType>(i);
            }
        }
    }
}