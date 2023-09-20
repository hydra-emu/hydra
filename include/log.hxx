#pragma once

#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <functional>
#include <str_hash.hxx>
#include <unordered_map>

using LoggingCallbacks =
    std::unordered_map<std::string, std::vector<std::function<void(const std::string&)>>>;

struct Logger
{
    template <typename... T>
    static void Fatal(fmt::format_string<T...> fmt, T&&... args)
    {
        std::string str = fmt::format(fmt, std::forward<T>(args)...);
        log_impl("Fatal", str);
    }

    template <typename... T>
    static void Warn(fmt::format_string<T...> fmt, T&&... args)
    {
        std::string str = fmt::format(fmt, std::forward<T>(args)...);
        log_impl("Warn", str);
    }

    template <typename... T>
    static void WarnOnce(fmt::format_string<T...> fmt, T&&... args)
    {
        std::unordered_map<uint32_t, bool>& warnings = get_warnings();

        std::string msg = fmt::format(fmt, std::forward<T>(args)...);
        uint32_t hash = str_hash(msg);
        if (warnings[hash])
            return;

        Logger::Warn("{}", msg);
        warnings[hash] = true;
    }

    template <typename... T>
    static void Info(fmt::format_string<T...> fmt, T&&... args)
    {
        std::string str = fmt::format(fmt, std::forward<T>(args)...);
        log_impl("Info", str);
    }

    template <typename... T>
    static void Debug([[maybe_unused]] fmt::format_string<T...> fmt, [[maybe_unused]] T&&... args)
    {
        std::string str = fmt::format(fmt, std::forward<T>(args)...);
        log_impl("Debug", str);
    }

    static void ClearWarnings()
    {
        get_warnings().clear();
    }

    static LoggingCallbacks& GetCallbacks()
    {
        static LoggingCallbacks callbacks;
        return callbacks;
    }

    static void HookCallback(const std::string& group,
                             std::function<void(const std::string&)> callback)
    {
        GetCallbacks()[group].push_back(callback);
    }

private:
    static inline void log_impl(const std::string& group, const std::string& message)
    {
        std::vector<std::function<void(const std::string&)>>& callbacks = GetCallbacks()[group];
        for (auto& callback : callbacks)
        {
            callback(message + "\n");
        }
    }

    static std::unordered_map<uint32_t, bool>& get_warnings()
    {
        static std::unordered_map<uint32_t, bool> warnings;
        return warnings;
    }
};
