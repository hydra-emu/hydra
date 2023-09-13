#pragma once

#include <error_factory.hxx>
#include <fstream>
#include <json.hpp>
#include <map>
#include <memory>
#include <mutex>
#include <string>

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

private:
    static std::map<std::string, std::string> map_;
    static std::string save_path_;

    static bool initialized_;
};
