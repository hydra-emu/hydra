#pragma once

#include <common/compatibility.hxx>
#include <error_factory.hxx>
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
        std::filesystem::directory_iterator it(Settings::Get("core_path"));
        std::filesystem::directory_iterator end;
        while (it != end)
        {
            if (it->path().extension() == ".so")
            {
                hydra::core_wrapper_t core(it->path());
                core.hc_get_emulator_info = reinterpret_cast<decltype(core.hc_get_emulator_info)>(
                    dlsym(core.dlhandle, "hc_get_emulator_info"));
                core_info info;
                info.path = it->path().string();
                info.core_name = core.hc_get_emulator_info(hc_emu_info::HC_INFO_CORE_NAME);
                info.system_name = core.hc_get_emulator_info(hc_emu_info::HC_INFO_SYSTEM_NAME);
                info.version = core.hc_get_emulator_info(hc_emu_info::HC_INFO_VERSION);
                info.author = core.hc_get_emulator_info(hc_emu_info::HC_INFO_AUTHOR);
                info.description = core.hc_get_emulator_info(hc_emu_info::HC_INFO_DESCRIPTION);
                info.extensions =
                    hydra::split(core.hc_get_emulator_info(hc_emu_info::HC_INFO_EXTENSIONS), ',');
                info.url = core.hc_get_emulator_info(hc_emu_info::HC_INFO_URL);
                info.license = core.hc_get_emulator_info(hc_emu_info::HC_INFO_LICENSE);
                CoreInfo.push_back(info);
            }

            ++it;
        }
    }

    static std::vector<core_info> CoreInfo;

private:
    static std::map<std::string, std::string> map_;
    static std::string save_path_;

    static bool initialized_;
    static bool core_info_initialized_;
};
