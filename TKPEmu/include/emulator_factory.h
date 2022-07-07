#ifndef TKP_EMUFACTORY_H
#define TKP_EMUFACTORY_H
#include <type_traits>
#include <filesystem>
#include <any>
#include "emulator.h"
#include "emulator_types.hxx"
#include <GameboyTKP/gb_tkpwrapper.h>
#include <N64TKP/n64_tkpwrapper.hxx>
#include <chip8/chip8_tkpwrapper.hxx>
#include <NESTKP/nes_tkpwrapper.hxx>
using EmulatorDataMap = std::array<EmulatorData, static_cast<int>(TKPEmu::EmuType::EmuTypeSize)>;
    using ExtensionMappings = std::unordered_map<std::string, TKPEmu::EmuType>;
namespace TKPEmu {
    class EmulatorFactory {
    private:
        static ExtensionMappings extension_mappings_;
        static EmulatorDataMap emulator_data_;
    public:
        static std::string GetSavePath();
        static std::shared_ptr<Emulator> Create(EmuType type);
        static EmuType GetEmulatorType(std::filesystem::path path);
        static const std::vector<std::string>& GetSupportedExtensions();
        static std::unique_ptr<OptionsBase> GetOptions(TKPEmu::EmuType type);
        static void SetEmulatorData(EmulatorDataMap map);
    };
}
#endif