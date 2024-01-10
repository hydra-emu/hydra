#include "hydra/core.hxx"
#define OPENSSL_API_COMPAT 10101
#include <corewrapper.hxx>
#include <filesystem>
#include <openssl/md5.h>
#include <scopeguard.hxx>
#include <settings.hxx>

namespace hydra
{
    EmulatorWrapper* EmulatorWrapper::instance = nullptr;

    EmulatorWrapper::EmulatorWrapper(IBase* shl, dynlib_handle_t hdl, void (*dfunc)(IBase*),
                                     const char* (*gfunc)(hydra::InfoType),
                                     const std::filesystem::path& core_path)
        : shell(shl), handle(hdl), destroy_function(dfunc), get_info_function(gfunc),
          core_path_(core_path)
    {
        instance = this;
        if (shell->hasInterface(hydra::InterfaceType::IConfigurable))
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
            if (core.path == core_path_)
            {
                found = true;
                core_name_ = core.core_name;
                core.last_played = std::time(nullptr);
                if (!core.required_files.empty())
                {
                    for (auto& required_file : core.required_files)
                    {
                        std::filesystem::path path =
                            Settings::Get(core.core_name + "_" + required_file);
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

        if (shell->hasInterface(hydra::InterfaceType::ICheat))
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
        static std::string buffer;
        std::string setting_key = instance->core_name_ + "_" + setting;
        buffer = Settings::Get(setting_key);
        return buffer.c_str();
    }

    void EmulatorWrapper::set_setting_wrapper(const char* setting, const char* value)
    {
        std::string setting_key = instance->core_name_ + "_" + setting;
        Settings::Set(setting_key, value);
    }
} // namespace hydra