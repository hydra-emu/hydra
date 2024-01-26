#include "hydra/core.hxx"
#include <cstdint>
#define OPENSSL_API_COMPAT 10101
#include <corewrapper.hxx>
#include <filesystem>
#include <openssl/md5.h>
#include <scopeguard.hxx>
#include <settings.hxx>

std::unordered_map<std::string, std::string> cached_settings;
#define has_i(x) (has[(size_t)hydra::InterfaceType::x])

namespace hydra
{
    EmulatorWrapper* EmulatorWrapper::instance = nullptr;

    EmulatorWrapper::EmulatorWrapper(IBase* shl, dynlib_handle_t hdl, void (*dfunc)(IBase*),
                                     const char* (*gfunc)(hydra::InfoType),
                                     const hydra::CoreInfo& core_info)
        : shell(shl), handle(hdl), destroy_function(dfunc), get_info_function(gfunc),
          core_info_(core_info)
    {
        instance = this;

        for (size_t i = 0; i < (size_t)hydra::InterfaceType::InterfaceCount; i++)
        {
            has[i] = shell->hasInterface((hydra::InterfaceType)i);
        }

        if (has_i(IConfigurable))
        {
            IConfigurable* config_interface = shell->asIConfigurable();
            config_interface->setGetCallback(get_setting_wrapper);
            config_interface->setSetCallback(set_setting_wrapper);
        }
    }

    EmulatorWrapper::~EmulatorWrapper()
    {
        destroy_function(shell);
        dynlib_close(handle);
    }

    bool EmulatorWrapper::LoadGame(const std::filesystem::path& path)
    {
        unsigned char result[MD5_DIGEST_LENGTH];
        std::ifstream file(path, std::ifstream::binary);
        MD5_CTX md5Context;
        MD5_Init(&md5Context);
        char buf[1024 * 16];
        while (file.good())
        {
            file.read(buf, sizeof(buf));
            MD5_Update(&md5Context, buf, file.gcount());
        }
        MD5_Final(result, &md5Context);
        std::stringstream md5stream;
        md5stream << std::hex << std::setfill('0');
        for (const auto& byte : result)
        {
            md5stream << std::setw(2) << (int)byte;
        }
        game_hash_ = md5stream.str();

        auto& cores = Settings::GetCoreInfo();
        bool found = false;
        for (auto& core : cores)
        {
            if (core_info_.path == core.path)
            {
                found = true;
                std::string core_name = core.core_name;
                core.last_played = std::time(nullptr);
                if (!core.required_files.empty())
                {
                    for (auto& required_file : core.required_files)
                    {
                        std::filesystem::path path = Settings::Get(core_name + "_" + required_file);
                        if (!std::filesystem::exists(path))
                        {
                            printf("Required file %s does not exist at path %s\n",
                                   required_file.c_str(), path.string().c_str());
                            return false;
                        }

                        shell->asIBase()->loadFile(required_file.c_str(), path.string().c_str());
                    }
                }
                break;
            }
        }

        if (!found)
        {
            return false;
        }

        bool ret = shell->asIBase()->loadFile("rom", path.string().c_str());

        if (has_i(ICheat))
        {
            init_cheats();
        }

        return ret;
    }

    const char* EmulatorWrapper::GetInfo(InfoType type)
    {
        return get_info_function(type);
    }

    void EmulatorWrapper::init_cheats()
    {
        if (!std::filesystem::create_directories(Settings::GetSavePath() / "cheats"))
        {
            if (!std::filesystem::exists(Settings::GetSavePath() / "cheats"))
            {
                printf("Failed to create cheats directory\n");
                return;
            }
        }

        // Check if this game already has saved cheats
        std::filesystem::path cheat_path =
            Settings::GetSavePath() / "cheats" / (game_hash_ + ".json");
        printf("cheat path: %s\n", cheat_path.c_str());
        if (std::filesystem::exists(cheat_path))
        {
            hydra::ICheat* cheat_interface = shell->asICheat();
            std::ifstream cheat_file(cheat_path);
            nlohmann::json cheat_json;
            cheat_file >> cheat_json;
            for (auto& cheat : cheat_json)
            {
                CheatMetadata cheat_metadata;
                std::vector<uint8_t> bytes = hydra::hex_to_bytes(cheat_metadata.code);
                cheat_metadata.handle = cheat_interface->addCheat(bytes.data(), bytes.size());

                if (cheat_metadata.handle != hydra::BAD_CHEAT)
                {
                    cheat_metadata.enabled = cheat["enabled"] == "true";
                    cheat_metadata.name = cheat["name"];
                    cheat_metadata.code = cheat["code"];

                    if (cheat_metadata.enabled)
                    {
                        cheat_interface->enableCheat(cheat_metadata.handle);
                    }
                    else
                    {
                        cheat_interface->disableCheat(cheat_metadata.handle);
                    }
                    cheats_.push_back(cheat_metadata);
                }
            }
        }
    }

    void EmulatorWrapper::save_cheats()
    {
        nlohmann::json cheat_json;
        for (const hydra::CheatMetadata& cheat : cheats_)
        {
            cheat_json.push_back({{"enabled", cheat.enabled ? "true" : "false"},
                                  {"name", cheat.name},
                                  {"code", cheat.code}});
        }
        std::ofstream cheat_file(Settings::GetSavePath() / "cheats" / (game_hash_ + ".json"));
        cheat_file << cheat_json.dump(4);
    }

    void EmulatorWrapper::ResetContext()
    {
        if (has_i(IOpenGlRendered))
        {
            shell->asIOpenGlRendered()->resetContext();
        }
    }

    void EmulatorWrapper::DestroyContext()
    {
        if (has_i(IOpenGlRendered))
        {
            shell->asIOpenGlRendered()->destroyContext();
        }
    }

    void EmulatorWrapper::SetFbo(GLuint fbo)
    {
        if (has_i(IOpenGlRendered))
        {
            shell->asIOpenGlRendered()->setFbo(fbo);
        }
    }

    void EmulatorWrapper::SetGetProcAddress(void* func)
    {
        if (has_i(IOpenGlRendered))
        {
            shell->asIOpenGlRendered()->setGetProcAddress(func);
        }
    }

    void EmulatorWrapper::RunFrame()
    {
        if (has_i(IFrontendDriven))
        {
            shell->asIFrontendDriven()->runFrame();
        }
    }

    uint16_t EmulatorWrapper::GetFps()
    {
        if (has_i(IFrontendDriven))
        {
            return shell->asIFrontendDriven()->getFps();
        }
        return 0;
    }

    void EmulatorWrapper::SetVideoCallback(void (*callback)(void* data, hydra::Size size))
    {
        if (has_i(ISoftwareRendered))
        {
            shell->asISoftwareRendered()->setVideoCallback(callback);
        }
    }

    hydra::PixelFormat EmulatorWrapper::GetPixelFormat()
    {
        if (has_i(ISoftwareRendered))
        {
            return shell->asISoftwareRendered()->getPixelFormat();
        }
        return hydra::PixelFormat::Invalid;
    }

    const CheatMetadata& EmulatorWrapper::GetCheat(uint32_t handle)
    {
        for (auto& cheat : cheats_)
        {
            if (cheat.handle == handle)
            {
                return cheat;
            }
        }
        static CheatMetadata empty;
        return empty;
    }

    const std::vector<CheatMetadata>& EmulatorWrapper::GetCheats()
    {
        return cheats_;
    }

    uint32_t EmulatorWrapper::EditCheat(const CheatMetadata& cheat, uint32_t old_handle)
    {
        hydra::scope_guard guard([this]() { save_cheats(); });

        if (old_handle == hydra::BAD_CHEAT)
        {
            ICheat* cheat_interface = shell->asICheat();
            std::vector<uint8_t> bytes = hydra::hex_to_bytes(cheat.code);
            uint32_t handle = cheat_interface->addCheat(bytes.data(), bytes.size());
            if (handle != hydra::BAD_CHEAT)
            {
                cheats_.push_back(cheat);
                cheats_.back().handle = handle;
                cheat_interface->disableCheat(handle);
                return handle;
            }
            return hydra::BAD_CHEAT;
        }
        else
        {
            for (auto& cheat_metadata : cheats_)
            {
                if (cheat_metadata.handle == old_handle)
                {
                    ICheat* cheat_interface = shell->asICheat();
                    cheat_interface->removeCheat(old_handle);
                    std::vector<uint8_t> bytes = hydra::hex_to_bytes(cheat.code);
                    uint32_t handle = cheat_interface->addCheat(bytes.data(), bytes.size());
                    if (handle != hydra::BAD_CHEAT)
                    {
                        cheat_metadata = cheat;
                        cheat_metadata.handle = handle;
                        return handle;
                    }
                    return hydra::BAD_CHEAT;
                }
            }
            return hydra::BAD_CHEAT;
        }
    }

    void EmulatorWrapper::RemoveCheat(uint32_t handle)
    {
        ICheat* cheat_interface = shell->asICheat();
        cheat_interface->removeCheat(handle);
        for (auto it = cheats_.begin(); it != cheats_.end(); ++it)
        {
            if (it->handle == handle)
            {
                cheats_.erase(it);
                save_cheats();
                return;
            }
        }
    }

    void EmulatorWrapper::EnableCheat(uint32_t handle)
    {
        ICheat* cheat_interface = shell->asICheat();
        cheat_interface->enableCheat(handle);
        for (auto& cheat : cheats_)
        {
            if (cheat.handle == handle)
            {
                cheat.enabled = true;
                save_cheats();
                return;
            }
        }
    }

    void EmulatorWrapper::DisableCheat(uint32_t handle)
    {
        ICheat* cheat_interface = shell->asICheat();
        cheat_interface->disableCheat(handle);
        for (auto& cheat : cheats_)
        {
            if (cheat.handle == handle)
            {
                cheat.enabled = false;
                save_cheats();
                return;
            }
        }
    }

    // We don't just direct to Settings::Get/Set because if we didn't prepend the core name
    // cores could access each other's settings which sounds like a security issue
    const char* EmulatorWrapper::get_setting_wrapper(const char* setting)
    {
        std::string setting_key = instance->core_info_.core_name + "_" + setting;
        if (cached_settings.find(setting_key) == cached_settings.end())
        {
            cached_settings[setting_key] = Settings::Get(setting_key);
        }
        return cached_settings[setting_key].c_str();
    }

    void EmulatorWrapper::set_setting_wrapper(const char* setting, const char* value)
    {
        std::string setting_key = instance->core_info_.core_name + "_" + setting;
        Settings::Set(setting_key, value);
        cached_settings[setting_key] = value;
    }

    std::shared_ptr<EmulatorWrapper> EmulatorFactory::Create(const std::string& path)
    {
        dynlib_handle_t handle = dynlib_open(path.c_str());

        if (!handle)
        {
            printf("Failed to load library %s: %s\n", path.c_str(), dynlib_get_error().c_str());
            return nullptr;
        }
        auto create_emu_p =
            (decltype(hydra::createEmulator)*)dynlib_get_symbol(handle, "createEmulator");
        if (!create_emu_p)
        {
            printf("Failed to find createEmulator in %s\n", path.c_str());
            dynlib_close(handle);
            return nullptr;
        }

        auto destroy_emu_p =
            (decltype(hydra::destroyEmulator)*)dynlib_get_symbol(handle, "destroyEmulator");

        if (!destroy_emu_p)
        {
            printf("Failed to find destroyEmulator in %s\n", path.c_str());
            dynlib_close(handle);
            return nullptr;
        }

        auto get_info_p = (decltype(hydra::getInfo)*)dynlib_get_symbol(handle, "getInfo");

        if (!get_info_p)
        {
            printf("Failed to find getInfo in %s\n", path.c_str());
            dynlib_close(handle);
            return nullptr;
        }

        const hydra::CoreInfo* info = nullptr;
        for (const auto& core : Settings::GetCoreInfo())
        {
            if (core.path == path)
            {
                info = &core;
            }
        }

        auto emulator = std::shared_ptr<EmulatorWrapper>(
            new EmulatorWrapper(create_emu_p(), handle, destroy_emu_p, get_info_p, *info));
        return emulator;
    }
} // namespace hydra