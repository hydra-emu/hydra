#pragma once

#include <algorithm>
#include <filesystem>
#include <functional>
#include <map>
#include <string_view>
#include <toml.hpp>
#include <unordered_set>
#include <vector>

#include <hydra/common/log.hxx>

namespace hydra::settings
{

    using toml_map = toml::basic_value<toml::preserve_comments, std::map>;

    std::vector<std::function<void(toml_map&)>>& getLoadFunctions();
    std::vector<std::function<void(toml_map&)>>& getSaveFunctions();

    std::filesystem::path configPath();

    void init();

    void poll();

    void setDirty();

    template <auto N>
    struct literal
    {
        constexpr literal(const char (&str)[N])
        {
            std::copy_n(str, N, value);
        }

        char value[N];
    };

    // using operator= on a Saveable will set the value and mark the settings file as dirty
    template <class T, literal g, literal n>
    struct Saveable
    {
        explicit Saveable() : value{}
        {
            getSaveFunctions().push_back([this](toml_map& toml) { save(toml); });
            getLoadFunctions().push_back([this](toml_map& toml) { load(toml); });

            // Sanity check to make sure we don't have duplicate settings
            static bool first = true;
            if (!first)
            {
                hydra::log("Duplicate setting: {}.{}", g.value, n.value);
            }
            first = false;
        }

        T& operator=(const T& other)
        {
            value = other;
            setDirty();
            return value;
        }

        operator T() const
        {
            return value;
        }

        T get()
        {
            return value;
        }

        // In case this is a vector or similar structure with push_back
        template <class U = T>
        void push_back(const typename U::value_type& val)
        {
            value.push_back(val);
            setDirty();
        }

        void save(toml_map& toml)
        {
            toml[group][name] = value;
        }

        void load(toml_map& toml)
        {
            if (toml.contains(group) && toml[group].contains(name))
                value = toml::get<T>(toml[group][name]);
            else
                hydra::log("Setting {} not found in config file", name);
        }

    private:
        T value;
        std::string group = g.value;
        std::string name = n.value;
    };

    struct Config
    {
        Saveable<int, "General", "Width"> windowWidth{};
        Saveable<int, "General", "Height"> windowHeight{};
        Saveable<std::vector<std::string>, "General", "LastContentPaths"> lastContentPaths{};
        Saveable<std::vector<std::string>, "General", "GameDirectories"> gameDirectories{};

        static Config& get()
        {
            static Config instance;
            return instance;
        }
    };

} // namespace hydra::settings