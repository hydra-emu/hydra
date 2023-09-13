#include <c8/c8_tkpwrapper.hxx>
#include <emulator_data.hxx>
#include <emulator_factory.hxx>
#include <emulator_settings.hxx>
#include <emulator_user_data.hxx>
#include <error_factory.hxx>
#include <iostream>
#include <n64/n64_hc.hxx>
#include <nes/nes_tkpwrapper.hxx>
#include <str_hash.hxx>

namespace hydra
{
    ExtensionMappings EmulatorFactory::extension_mappings_{};

    std::string EmulatorFactory::GetSavePath()
    {
        static std::string dir;
        if (dir.empty())
        {
#if defined(__linux__)
            dir = getenv("HOME") + std::string("/.config/hydra/");
#elif defined(_WIN32)
            dir = getenv("APPDATA") + std::string("/hydra/");
#elif defined(__APPLE__)
            dir = getenv("HOME") + std::string("/Library/Application Support/hydra/");
#endif
            if (dir.empty())
            {
                throw ErrorFactory::generate_exception(
                    __func__, __LINE__, "GetSavePath was not defined for this environment");
            }
            if (!std::filesystem::create_directories(dir))
            {
                if (std::filesystem::exists(dir))
                {
                    return dir;
                }
                throw ErrorFactory::generate_exception(__func__, __LINE__,
                                                       "Failed to create directories");
            }
        }
        return dir;
    }

    // TODO: detect emutype by magic bytes instead of extension
    EmuType EmulatorFactory::GetEmulatorType(std::filesystem::path path)
    {
        auto extension = path.extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        if (extension_mappings_.empty())
        {
            for (int i = 0; i < EmuTypeSize; ++i)
            {
                auto& data = EmulatorSettings::GetEmulatorData(static_cast<EmuType>(i));
                for (auto& ext : data.Extensions)
                {
                    extension_mappings_[ext] = static_cast<EmuType>(i);
                }
            }
        }
        try
        {
            return extension_mappings_.at(extension);
        } catch (...)
        {
            throw ErrorFactory::generate_exception(__func__, __LINE__, "Unknown file extension");
        }
    }

    std::unique_ptr<Core> EmulatorFactory::Create(EmuType type)
    {
        std::unique_ptr<Core> emulator;
        switch (type)
        {
            case EmuType::Gameboy:
            {
                // emulator = std::make_unique<hydra::HydraCore_Gameboy>();
                break;
            }
            case EmuType::N64:
            {
                emulator = std::make_unique<hydra::HydraCore_N64>();
                auto ipl_path =
                    EmulatorSettings::GetEmulatorData(hydra::EmuType::N64).UserData.Get("IPLPath");
                if (!emulator->LoadFile("ipl", ipl_path))
                {
                    throw ErrorFactory::generate_exception(__func__, __LINE__,
                                                           "Failed to load IPL");
                }
                break;
            }
            // case EmuType::c8:
            // {
            //     break;
            // }
            // case EmuType::NES:
            // {
            //     break;
            // }
            default:
            {
                throw ErrorFactory::generate_exception(__func__, __LINE__,
                                                       "EmulatorFactory::Create failed");
            }
        }
        return emulator;
    }
} // namespace hydra