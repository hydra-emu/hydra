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
        using ExtensionMappings = std::unordered_map<std::string, EmuType>;
        static const ExtensionMappings& get_extension_mappings();
    public:
        static std::shared_ptr<Emulator> Create(EmuType type, std::any args = {});
        static void LoadEmulatorTools(std::vector<std::unique_ptr<IMApplication>>& tools, std::shared_ptr<Emulator> emulator, EmuType emu_type);
        static void LoadGenericTools(std::vector<std::unique_ptr<TKPEmu::Applications::Drawable>>& tools, std::shared_ptr<Emulator> emulator, EmuType emu_type);
        static EmuType GetEmulatorType(std::filesystem::path path);
        static const std::vector<std::string>& GetSupportedExtensions();
    };
}
#endif