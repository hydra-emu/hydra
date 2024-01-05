#pragma once

#include <compatibility.hxx>
#include <corewrapper.hxx>
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <hsystem.hxx>
#include <hydra/core.hxx>
#include <json.hpp>
#include <log.hxx>
#include <map>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>

struct CoreInfo
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
    std::vector<std::string> extensions;
    unsigned int icon_texture = 0;
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
        map().clear();
        save_path() = path;
        std::string data = hydra::fread(path);
        if (data.empty())
        {
            return;
        }

        json j_map;
        try
        {
            j_map = json::parse(data);
        } catch (std::exception& e)
        {
            hydra::log("Failed to parse settings file: {}", e.what());
            return;
        }
        map() = j_map.get<std::map<std::string, std::string>>();
    }

    static std::string Get(const std::string& key)
    {
        if (map().find(key) == map().end())
        {
            Set(key, "");
            return "";
        }

        return map()[key];
    }

    static void Set(const std::string& key, const std::string& value)
    {
        map()[key] = value;
        json j_map(map());
        std::string data = j_map.dump(4);
        hydra::fwrite(save_path(), std::span<const uint8_t>((uint8_t*)data.data(), data.size()));
    }

    static bool IsEmpty()
    {
        return map().empty();
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
#elif defined(HYDRA_WEB)
            dir = std::filesystem::path("/hydra");
#endif
            if (dir.empty())
            {
                throw std::runtime_error("GetSavePath was not defined for this environment");
            }

            if (!std::filesystem::create_directories(dir))
            {
                if (std::filesystem::exists(dir))
                {
                    return dir;
                }
                throw std::runtime_error("Failed to create directories");
            }
        }
        return dir;
    }

    static std::filesystem::path GetCachePath()
    {
        static std::filesystem::path dir;
        if (dir.empty())
        {
#if defined(HYDRA_LINUX) || defined(HYDRA_FREEBSD)
            dir = std::filesystem::path(getenv("HOME")) / ".cache" / "hydra";
#elif defined(HYDRA_WINDOWS)
            dir = std::filesystem::path(getenv("APPDATA")) / "hydra" / "cache";
#elif defined(HYDRA_MACOS)
            dir = std::filesystem::path(getenv("HOME")) / "Library" / "Caches" / "hydra" / "cache";
#else
            dir = GetSavePath() / "cache";
#endif
        }

        if (dir.empty())
        {
            throw std::runtime_error("GetCachePath was not defined for this environment");
        }

        if (!std::filesystem::create_directories(dir))
        {
            if (std::filesystem::exists(dir))
            {
                return dir;
            }
            throw std::runtime_error("Failed to create cache");
        }

        return dir;
    }

    static void InitCoreInfo()
    {
        if (core_info_initialized())
            return;
        core_info_initialized() = true;
        if (Settings::Get("core_path").empty())
        {
            Settings::Set("core_path", (Settings::GetSavePath() / "cores").string());
        }

        if (!std::filesystem::exists(Settings::Get("core_path")))
        {
            printf("Failed to find core info: %s\n", Settings::Get("core_path").c_str());
            return;
        }

        std::filesystem::directory_iterator it(Settings::Get("core_path"));
        std::filesystem::directory_iterator end;
        while (it != end)
        {
            if (it->path().extension() == hydra::dynlib_get_extension())
            {
                // TODO: cache these to a json or whatever so we don't dlopen every time
                void* handle = hydra::dynlib_open(it->path().string().c_str());

                struct Destructor
                {
                    void* handle;

                    ~Destructor()
                    {
                        hydra::dynlib_close(handle);
                    }
                };

                Destructor d{handle};

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
                    hydra::log("Could not find symbol getInfo in core {}",
                               it->path().string().c_str());
                    ++it;
                    continue;
                }
                struct CoreInfo info;
                info.path = it->path().string();
                info.core_name = get_info_p(hydra::InfoType::CoreName);
                info.system_name = get_info_p(hydra::InfoType::SystemName);
                info.version = get_info_p(hydra::InfoType::Version);
                info.author = get_info_p(hydra::InfoType::Author);
                info.description = get_info_p(hydra::InfoType::Description);
                info.extensions = hydra::split(get_info_p(hydra::InfoType::Extensions), ',');
                info.url = get_info_p(hydra::InfoType::Website);
                info.license = get_info_p(hydra::InfoType::License);
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
                GetCoreInfo().push_back(info);
            }

            ++it;
        }
    }

    static void ReinitCoreInfo()
    {
        core_info_initialized() = false;
        GetCoreInfo().clear();
        InitCoreInfo();
    }

    static std::string Print()
    {
        std::string ret;
        ret += fmt::format("version: {}\n", HYDRA_VERSION);
        ret += fmt::format("system: {}\n", hydra_os());
        ret += fmt::format("settings:\n{}\n", json(map()).dump(4));
        return ret;
    }

    static std::vector<CoreInfo>& GetCoreInfo()
    {
        static std::vector<struct CoreInfo> c;
        return c;
    }

    static void InitCoreIcons();

private:
    static std::map<std::string, std::string>& map()
    {
        static std::map<std::string, std::string> m;
        return m;
    }

    static std::filesystem::path& save_path()
    {
        static std::filesystem::path p;
        return p;
    }

    static bool& core_info_initialized()
    {
        static bool b;
        return b;
    }
};
