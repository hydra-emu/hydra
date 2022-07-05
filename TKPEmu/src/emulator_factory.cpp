#include <iostream>
#include <lib/str_hash.h>
#include <include/emulator_factory.h>
#include <include/error_factory.hxx>
#include <include/start_options.hxx>

namespace TKPEmu {
    // TODO: detect emutype by magic bytes instead of extension
    EmuType EmulatorFactory::GetEmulatorType(std::filesystem::path path) {
        auto ext = path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(),
            [](unsigned char c){ return std::tolower(c); });
        auto& extension_mappings = get_extension_mappings();
        try {
            return extension_mappings.at(ext);
        } catch (...) {
            throw ErrorFactory::generate_exception(__func__, __LINE__, "Unknown file extension");
        }
    }
    const EmulatorFactory::ExtensionMappings& EmulatorFactory::get_extension_mappings() {
        static ExtensionMappings extension_mappings_ = {
            { ".gb", EmuType::Gameboy },
            { ".gbc", EmuType::Gameboy },
            { ".nes", EmuType::NES },
            { ".n64", EmuType::N64 },
            { ".z64", EmuType::N64 },
            { ".ch8", EmuType::Chip8 },
        };
        return extension_mappings_;
    }
    const std::vector<std::string>& EmulatorFactory::GetSupportedExtensions() {
        static std::vector<std::string> supported_extensions_;
        if (supported_extensions_.empty()) {
            auto& extension_mappings = get_extension_mappings();
            for (const auto& item : extension_mappings) {
                supported_extensions_.push_back(item.first);
            }
        }
        return supported_extensions_;
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
            case EmuType::None:
            default: {
                return nullptr;
            }
        }
    }
}