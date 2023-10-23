#pragma once

#include "log.h"
#include <compatibility.hxx>
#include <core_loader.hxx>
#include <error_factory.hxx>
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <hydra/core.hxx>
#include <json.hpp>
#include <map>
#include <memory>
#include <mutex>
#include <string>

struct EmulatorInfo
{
    std::string path;
    std::string core_name;
    std::string system_name;
    std::string author;
    std::string version;
    std::string description;
    std::string license;
    std::string url;
    int max_players;
    std::vector<std::string> firmware_files;
    std::vector<std::string> extensions;
};

// Essentially a wrapper around a std::map<std::string, std::string> that locks a mutex
// upon access and has a save function that saves as a json to file
class Settings
{
    using json = nlohmann::json;
    Settings() = delete;
    ~Settings() = delete;

public:
    static void Open(const std::filesystem::path& path)
    {
        save_path_ = path;
        std::ifstream ifs(save_path_);
        if (ifs.good())
        {
            json j_map;
            ifs >> j_map;
            map_ = j_map.get<std::map<std::string, std::string>>();
        }
        initialized_ = true;
    }

    static std::string Get(const std::string& key)
    {
        if (!initialized_)
        {
            throw ErrorFactory::generate_exception(__func__, __LINE__, "Settings not initialized");
        }

        if (map_.find(key) == map_.end())
        {
            Set(key, "");
            return "";
        }

        return map_[key];
    }

    static void Set(const std::string& key, const std::string& value)
    {
        if (!initialized_)
        {
            throw ErrorFactory::generate_exception(__func__, __LINE__, "Settings not initialized");
        }

        map_[key] = value;
        std::ofstream ofs(save_path_, std::ios::trunc);
        json j_map(map_);
        ofs << j_map << std::endl;
    }

    static bool IsEmpty()
    {
        if (!initialized_)
        {
            throw ErrorFactory::generate_exception(__func__, __LINE__, "Settings not initialized");
        }

        return map_.empty();
    }

    static std::filesystem::path GetSavePath()
    {
        static std::filesystem::path dir;
        if (dir.empty())
        {
#if defined(HYDRA_LINUX) || defined(HYDRA_FREEBSD)
            dir = std::filesystem::path(getenv("HOME")) / ".config" / "hydra";
#elif defined(HYDRA_WINDOWS)
            dir = std::filesystem::path(getenv("APPDATA")) / "hydra";
#elif defined(HYDRA_MACOS)
            dir =
                std::filesystem::path(getenv("HOME")) / "Library" / "Application Support" / "hydra";
#elif defined(HYDRA_ANDROID)
            std::ifstream cmdline("/proc/self/cmdline");
            std::string applicationName;
            std::getline(cmdline, applicationName, '\0');
            dir = std::filesystem::path("/data") / "data" / applicationName / "files";
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

    static void InitCoreInfo()
    {
        if (core_info_initialized_)
            return;
        core_info_initialized_ = true;
        if (Settings::Get("core_path").empty())
        {
            Settings::Set("core_path", (std::filesystem::current_path()).string());
        }
        std::filesystem::directory_iterator it(Settings::Get("core_path"));
        std::filesystem::directory_iterator end;
        while (it != end)
        {
            if (it->path().extension() == hydra::dynlib_get_extension())
            {
                // TODO: cache these to a json or whatever so we don't dlopen every time
                void* handle = hydra::dynlib_open(it->path().string().c_str());

                if (!handle)
                {
                    printf("%s\n", hydra::dynlib_get_error().c_str());
                    ++it;
                    continue;
                }

                auto get_info_p =
                    (decltype(hydra::getInfo)*)hydra::dynlib_get_symbol(handle, "getInfo");
                if (!get_info_p)
                {
                    log_warn(
                        fmt::format("Could not find symbol getInfo in core {}", it->path().string())
                            .c_str());
                    ++it;
                    continue;
                }
                EmulatorInfo info;
                info.path = it->path().string();
                info.core_name = get_info_p(hydra::InfoType::CoreName);
                info.system_name = get_info_p(hydra::InfoType::SystemName);
                info.version = get_info_p(hydra::InfoType::Version);
                info.author = get_info_p(hydra::InfoType::Author);
                info.description = get_info_p(hydra::InfoType::Description);
                info.extensions = hydra::split(get_info_p(hydra::InfoType::Extensions), ',');
                info.url = get_info_p(hydra::InfoType::Website);
                info.license = get_info_p(hydra::InfoType::License);
                info.firmware_files = hydra::split(get_info_p(hydra::InfoType::Firmware), ',');
                info.max_players = 1;

                bool one_active = false;
                for (int i = 1; i <= info.max_players; i++)
                {
                    std::string active = Settings::Get(info.core_name + "_controller_" +
                                                       std::to_string(i) + "_active");
                    if (active == "true")
                    {
                        one_active = true;
                    }
                    else if (active.empty())
                    {
                        Settings::Set(info.core_name + "_controller_" + std::to_string(i) +
                                          "_active",
                                      "false");
                    }
                }
                if (!one_active)
                {
                    Settings::Set(info.core_name + "_controller_1_active", "true");
                }
                CoreInfo.push_back(info);
            }

            ++it;
        }
    }

    static void ReinitCoreInfo()
    {
        core_info_initialized_ = false;
        CoreInfo.clear();
        InitCoreInfo();
    }

    static std::vector<EmulatorInfo> CoreInfo;

private:
    static std::map<std::string, std::string> map_;
    static std::filesystem::path save_path_;

    static bool initialized_;
    static bool core_info_initialized_;
};
