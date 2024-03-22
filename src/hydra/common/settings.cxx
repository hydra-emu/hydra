#include <atomic>
#include <hydra/common/settings.hxx>
#include <map>
#include <toml.hpp>

std::atomic_bool dirty = false;

static void save()
{
    std::filesystem::path path = hydra::settings::configPath() / "config.toml";

    toml::basic_value<toml::preserve_comments, std::map> data;
    for (auto& saveable : hydra::settings::getSaveFunctions())
    {
        saveable(data);
    }

    std::ofstream file(path, std::ios::out);
    file << data;
    file.close();
}

static void load()
{
    std::filesystem::path path = hydra::settings::configPath() / "config.toml";
    if (!std::filesystem::exists(path))
    {
        return;
    }

    toml::basic_value<toml::preserve_comments, std::map> data = toml::parse(path);
    for (auto& loadable : hydra::settings::getLoadFunctions())
    {
        loadable(data);
    }
}

namespace hydra::settings
{

    std::vector<std::function<void(toml_map&)>>& getLoadFunctions()
    {
        static std::vector<std::function<void(toml_map&)>> loadFunctions;
        return loadFunctions;
    }

    std::vector<std::function<void(toml_map&)>>& getSaveFunctions()
    {
        static std::vector<std::function<void(toml_map&)>> saveFunctions;
        return saveFunctions;
    }

    std::filesystem::path configPath()
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

    void init()
    {
        Config::get();
        load();
        std::atexit(poll);
    }

    void poll()
    {
        if (dirty.exchange(false))
        {
            save();
        }
    }

    void setDirty()
    {
        dirty = true;
    }
} // namespace hydra::settings