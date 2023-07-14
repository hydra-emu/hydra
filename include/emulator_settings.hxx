#pragma once

#include <emulator_data.hxx>
using EmulatorDataMap = std::array<EmulatorData, EmuTypeSize>;
using ExtensionMappings = std::unordered_map<std::string, hydra::EmuType>;
using GeneralSettings = EmulatorUserData;

struct EmulatorSettings
{
  private:
    static EmulatorDataMap emulator_data_;
    static GeneralSettings general_settings_;

  public:
    static EmulatorData& GetEmulatorData(hydra::EmuType type)
    {
        return emulator_data_.at(static_cast<int>(type));
    }

    static GeneralSettings& GetGeneralSettings()
    {
        return general_settings_;
    }
};
