#ifndef TKP_EMUFACTORY_H
#define TKP_EMUFACTORY_H
#include <type_traits>
#include <filesystem>
#include <any>
#include "emulator.h"
#include "base_application.h"
#include "emulator_types.hxx"
#include <include/generic_drawable.h>
#include <GameboyTKP/gb_tkpwrapper.h>
#include <N64TKP/n64_tkpwrapper.hxx>
#include <chip8/chip8_tkpwrapper.hxx>

namespace TKPEmu {
    class EmulatorFactory {
    private:
        using IMApplication = TKPEmu::Applications::IMApplication;
    public:
        static std::shared_ptr<Emulator> Create(EmuType type, std::any args = {}) { 
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
        static void LoadEmulatorTools(std::vector<std::unique_ptr<IMApplication>>& tools, std::shared_ptr<Emulator> emulator, EmuType emu_type);
        static void LoadGenericTools(std::vector<std::unique_ptr<TKPEmu::Applications::Drawable>>& tools, std::shared_ptr<Emulator> emulator, EmuType emu_type);
        static EmuType GetEmulatorType(std::filesystem::path path);
    };
}
#endif