#ifndef TKP_EMUFACTORY_H
#define TKP_EMUFACTORY_H
#include <type_traits>
#include <filesystem>
#include <any>
#include <include/emulator.h>
#include <include/emulator_types.hxx>

using ExtensionMappings = std::unordered_map<std::string, hydra::EmuType>;
namespace hydra {
    class EmulatorFactory {
    private:
        static ExtensionMappings extension_mappings_;
    public:
        static std::string GetSavePath();
        static std::shared_ptr<Emulator> Create(EmuType type);
        static EmuType GetEmulatorType(std::filesystem::path path);
        static const std::vector<std::string>& GetSupportedExtensions();
    };
}
#endif