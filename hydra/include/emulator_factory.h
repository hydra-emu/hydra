#ifndef TKP_EMUFACTORY_H
#define TKP_EMUFACTORY_H
#include <type_traits>
#include <filesystem>
#include <any>
#include <include/emulator.h>
#include <include/emulator_user_data.hxx>
#include <include/emulator_types.hxx>
#include <GameboyTKP/gb_tkpwrapper.h>
#include <N64TKP/n64_tkpwrapper.hxx>
#include <chip8/chip8_tkpwrapper.hxx>
#include <NESTKP/nes_tkpwrapper.hxx>
// Map for constant emulator data like screen size
using EmulatorDataMap = std::array<EmulatorData, static_cast<int>(TKPEmu::EmuType::EmuTypeSize)>;
// Map for variable emulator data like firmware path
using EmulatorUserDataMap = std::array<EmulatorUserData, static_cast<int>(TKPEmu::EmuType::EmuTypeSize)>;
using ExtensionMappings = std::unordered_map<std::string, TKPEmu::EmuType>;
namespace TKPEmu {
    class EmulatorFactory {
    private:
        static ExtensionMappings extension_mappings_;
        static EmulatorDataMap emulator_data_;
        static EmulatorUserDataMap emulator_user_data_;
    public:
        static std::string GetSavePath();
        static std::shared_ptr<Emulator> Create(EmuType type);
        static EmuType GetEmulatorType(std::filesystem::path path);
        static const std::vector<std::string>& GetSupportedExtensions();
        static KeyMappings GetMappings(TKPEmu::EmuType type);
        static void SetEmulatorData(EmulatorDataMap map);
        static const EmulatorDataMap& GetEmulatorData() { return emulator_data_; }
        static void SetEmulatorUserData(EmulatorUserDataMap map);
        static EmulatorUserDataMap& GetEmulatorUserData() { return emulator_user_data_; }
    };
}
#endif