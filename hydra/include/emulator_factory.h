#ifndef TKP_EMUFACTORY_H
#define TKP_EMUFACTORY_H
#include <type_traits>
#include <filesystem>
#include <any>
#include <include/emulator.h>
#include <include/emulator_user_data.hxx>
#include <include/emulator_types.hxx>

// Map for constant emulator data like screen size
using EmulatorDataMap = std::array<EmulatorData, static_cast<int>(TKPEmu::EmuType::EmuTypeSize)>;
// Map for variable emulator data like firmware path
using EmulatorUserDataMap = std::array<EmulatorUserData, static_cast<int>(TKPEmu::EmuType::EmuTypeSize)>;
using ExtensionMappings = std::unordered_map<std::string, TKPEmu::EmuType>;
using GeneralSettings = EmulatorUserData;
namespace TKPEmu {
    class EmulatorFactory {
    private:
        static ExtensionMappings extension_mappings_;
        static EmulatorDataMap emulator_data_;
        static EmulatorUserDataMap emulator_user_data_;
        static GeneralSettings settings_;
    public:
        static std::string GetSavePath();
        static std::shared_ptr<Emulator> Create(EmuType type);
        static EmuType GetEmulatorType(std::filesystem::path path);
        static const std::vector<std::string>& GetSupportedExtensions();
        static KeyMappings& GetMappings(TKPEmu::EmuType type);
        static GeneralSettings& GetGeneralSettings() { return settings_; }
        static void SetGeneralSettings(GeneralSettings settings) { settings_ = std::move(settings); }
        static void SetEmulatorData(EmulatorDataMap map);
        static const EmulatorDataMap& GetEmulatorData() { return emulator_data_; }
        static void SetEmulatorUserData(EmulatorUserDataMap map);
        static EmulatorUserDataMap& GetEmulatorUserData() { return emulator_user_data_; }
    };
}
#endif