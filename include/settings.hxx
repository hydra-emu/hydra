#pragma once

#include <common/compatibility.hxx>
#include <error_factory.hxx>
#include <fmt/format.h>
#include <fstream>
#include <json.hpp>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <ui_common.hxx>

struct core_info
{
    std::string path;
    std::string core_name;
    std::string system_name;
    std::string author;
    std::string version;
    std::string description;
    std::string license;
    std::string url;
    std::string firmware_files;
    int max_players;
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
    static void Open(const std::string& path)
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
            if (it->path().extension() == dynlib_get_extension())
            {
                hydra::core_wrapper_t core(it->path());
                core.hc_get_info_p = reinterpret_cast<decltype(core.hc_get_info_p)>(
                    dynlib_get_symbol(core.dl_handle, "hc_get_info"));
                if (!core.hc_get_info_p)
                {
                    log_warn(fmt::format("Could not find symbol hc_get_info in core {}",
                                         it->path().string())
                                 .c_str());
                    continue;
                }
                core_info info;
                info.path = it->path().string();
                info.core_name = core.hc_get_info_p(hc_info::HC_INFO_CORE_NAME);
                info.system_name = core.hc_get_info_p(hc_info::HC_INFO_SYSTEM_NAME);
                info.version = core.hc_get_info_p(hc_info::HC_INFO_VERSION);
                info.author = core.hc_get_info_p(hc_info::HC_INFO_AUTHOR);
                info.description = core.hc_get_info_p(hc_info::HC_INFO_DESCRIPTION);
                info.extensions =
                    hydra::split(core.hc_get_info_p(hc_info::HC_INFO_EXTENSIONS), ',');
                info.url = core.hc_get_info_p(hc_info::HC_INFO_URL);
                info.license = core.hc_get_info_p(hc_info::HC_INFO_LICENSE);
                info.firmware_files = core.hc_get_info_p(hc_info::HC_INFO_FIRMWARE_FILES);
                info.max_players = std::atoi(core.hc_get_info_p(hc_info::HC_INFO_MAX_PLAYERS));
                if (info.max_players == 0)
                {
                    info.max_players = 1;
                    log_warn("Could not get HC_INFO_MAX_PLAYERS for core, setting to 1");
                }

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

    static std::vector<core_info> CoreInfo;

private:
    static std::map<std::string, std::string> map_;
    static std::string save_path_;

    static bool initialized_;
    static bool core_info_initialized_;
};
