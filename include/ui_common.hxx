#pragma once

#include <array>
#include <emulator_types.hxx>
#include <filesystem>
#include <vector>

namespace hydra
{
    struct emulator_data_t
    {
        std::string Name;
        std::vector<std::string> Extensions;
    };
    class Core;

    class UiCommon
    {
    public:
        static std::string GetSavePath();
        static std::unique_ptr<Core> Create(EmuType type);
        static hydra::EmuType GetEmulatorType(const std::filesystem::path& path);
        static std::array<emulator_data_t, EmuTypeSize> EmulatorData;
    };
} // namespace hydra
