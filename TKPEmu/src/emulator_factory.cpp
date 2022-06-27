#include <iostream>
#include <lib/str_hash.h>
#include <include/emulator_factory.h>
#include <include/error_factory.hxx>
#include <include/generic_tool_make.h>
#include <GameboyTKP/gb_disassembler.h>
#include <GameboyTKP/gb_tracelogger.h>
#include <GameboyTKP/gb_romdata.h>
#include <N64TKP/n64_tkpromdisassembly.hxx>

namespace TKPEmu {
    void EmulatorFactory::LoadEmulatorTools(std::vector<std::unique_ptr<IMApplication>>& tools, std::shared_ptr<Emulator> emulator, EmuType emu_type) {
        switch (emu_type) {
            case EmuType::Gameboy: {
                tools.push_back(std::make_unique<Applications::GameboyRomData>("Rom data", "Gameboy rom data"));
                tools.push_back(std::make_unique<Applications::GameboyDisassembler>("Disassembler", "Gameboy disassembler"));
                tools.push_back(std::make_unique<Applications::GameboyTracelogger>("Tracelogger", "Gameboy tracelogger"));
                break;
            }
            case EmuType::N64: {
                tools.push_back(std::make_unique<Applications::N64_RomDisassembly>("Rom disassembly", "N64 rom disassembly"));
                break;
            }
            default: {
                std::cerr << "LoadEmulatorTools was not implemented for this emulator\nNo tools loaded..." << std::endl;
                break;
            }
        }
        for (const auto& app : tools) {
            app->SetEmulator(emulator);
        }
    }
    void EmulatorFactory::LoadGenericTools(std::vector<std::unique_ptr<TKPEmu::Applications::Drawable>>& tools, std::shared_ptr<Emulator> emulator, EmuType emu_type) {
        tools.push_back(MakeGenericTool<TKPEmu::Applications::Type::GenericDisassembler>(emu_type, emulator));
    }
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
    std::shared_ptr<Emulator> EmulatorFactory::Create(EmuType type, std::any args) { 
        switch (type) {
            case EmuType::Gameboy: {
                return std::make_shared<Gameboy::Gameboy>(args);
            }
            case EmuType::N64: {
                return std::make_shared<N64::N64_TKPWrapper>(args);
            }
            case EmuType::Chip8: {
                return std::make_shared<Chip8::Chip8>(args);
            }
            case EmuType::None:
            default: {
                return nullptr;
            }
        }
    }
}