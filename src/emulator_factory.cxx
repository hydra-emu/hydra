#include <c8/c8_tkpwrapper.hxx>
#include <emulator_data.hxx>
#include <emulator_factory.hxx>
#include <emulator_settings.hxx>
#include <emulator_user_data.hxx>
#include <error_factory.hxx>
#include <gb/gb_tkpwrapper.hxx>
#include <iostream>
#include <n64/n64_tkpwrapper.hxx>
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
#endif
            if (dir.empty())
            {
                throw ErrorFactory::generate_exception(
                    __func__, __LINE__, "GetSavePath was not defined for this environment");
            }
            if (!std::filesystem::create_directories(dir))
            {
                if (std::filesystem::exists(dir))
                    return dir;
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

    std::shared_ptr<Emulator> EmulatorFactory::Create(EmuType type)
    {
        const auto& data = EmulatorSettings::GetEmulatorData(type);
        std::shared_ptr<Emulator> emulator;
        switch (type)
        {
            case EmuType::Gameboy:
            {
                emulator = std::make_shared<Gameboy::Gameboy_TKPWrapper>();
                break;
            }
            case EmuType::N64:
            {
                emulator = std::make_shared<N64::N64_TKPWrapper>();
                break;
            }
            case EmuType::c8:
            {
                emulator = std::make_shared<c8::Chip8_TKPWrapper>();
                break;
            }
            case EmuType::NES:
            {
                emulator = std::make_shared<NES::NES_TKPWrapper>();
                break;
            }
            default:
            {
                throw ErrorFactory::generate_exception(__func__, __LINE__,
                                                       "EmulatorFactory::Create failed");
            }
        }
        emulator->SetWidth(data.DefaultWidth);
        emulator->SetHeight(data.DefaultHeight);
        return emulator;
    }
} // namespace hydra