#ifndef TKP_EMUFACTORY_H
#define TKPEMU_EMUFACTORY_H
#include <type_traits>
#include <filesystem>
#include "emulator.h"
#include "base_application.h"
#include "../GameboyTKP/gameboy.h"
namespace TKPEmu {
    enum class EmuType {
        None,
        Gameboy,
    };
    class EmulatorFactory {
    private:
        using IMApplication = TKPEmu::Applications::IMApplication;
    public:
        template<class... Args>
        static std::unique_ptr<Emulator> Create(EmuType type, Args&... args) { 
            switch (type) {
                case EmuType::Gameboy: {
                    return std::make_unique<Gameboy::Gameboy>(args...);
                }
                case EmuType::None:
                default: {
                    return nullptr;
                }
            }
        }
        static void LoadEmulatorTools(std::vector<std::unique_ptr<IMApplication>>& tools, Emulator* emulator, EmuType emu_type);
        static EmuType GetEmulatorType(std::filesystem::path path);
    };
}
#endif