#include <core.hxx>
#include <error_factory.hxx>
#include <iostream>
#include <n64/n64_hc.hxx>
#include <settings.hxx>
#include <str_hash.hxx>
#include <ui_common.hxx>

namespace hydra
{
    std::array<emulator_data_t, EmuTypeSize> UiCommon::EmulatorData;

    std::string UiCommon::GetSavePath()
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

    std::unique_ptr<Core> UiCommon::Create(EmuType type)
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
                auto ipl_path = Settings::Get("n64_ipl_path");
                if (ipl_path.empty())
                {
                    throw ErrorFactory::generate_exception(__func__, __LINE__,
                                                           "IPL path was not set");
                }

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

    hydra::EmuType UiCommon::GetEmulatorType(const std::filesystem::path& path)
    {
        for (int i = 0; i < EmuTypeSize; i++)
        {
            const auto& emulator_data = EmulatorData[i];
            for (const auto& str : emulator_data.Extensions)
            {
                if (path.extension() == str)
                {
                    return static_cast<hydra::EmuType>(i);
                }
            }
        }

        return hydra::EmuType::EmuTypeSize;
    }
} // namespace hydra