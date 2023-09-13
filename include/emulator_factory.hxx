#pragma once

#include <any>
#include <core.hxx>
#include <emulator_types.hxx>
#include <filesystem>
#include <type_traits>
#include <unordered_map>

using ExtensionMappings = std::unordered_map<std::string, hydra::EmuType>;

namespace hydra
{
    class EmulatorFactory
    {
    private:
        static ExtensionMappings extension_mappings_;

    public:
        static std::string GetSavePath();
        static std::unique_ptr<Core> Create(EmuType type);
        static EmuType GetEmulatorType(std::filesystem::path path);
        static const std::vector<std::string>& GetSupportedExtensions();
    };
} // namespace hydra
