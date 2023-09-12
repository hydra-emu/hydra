#pragma once

#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <str_hash.hxx>
#include <unordered_map>

// TODO: move to cpp file
#define ENABLE_LOGGING true
#define ENABLE_DEBUG 0
#define RSP_LOGGING false
#define CPU_LOGGING false

struct Logger
{
    template <typename... T>
    static void Fatal(fmt::format_string<T...> fmt, T&&... args)
    {
        std::string str = fmt::format(fmt, std::forward<T>(args)...);
        printf("%s\n", str.c_str());
        exit(1);
    }

    template <typename... T>
    static void Warn(fmt::format_string<T...> fmt, T&&... args)
    {
        std::string str = fmt::format(fmt, std::forward<T>(args)...);
        log_impl<LogWarning, ConsoleNormal>(str);
    }

    template <typename... T>
    static void WarnOnce(fmt::format_string<T...> fmt, T&&... args)
    {
        static std::unordered_map<uint32_t, bool> warnings = get_warnings();

        std::string msg = fmt::format(fmt, std::forward<T>(args)...);
        uint32_t hash = str_hash(msg);
        if (warnings[hash])
            return;

        Logger::Warn("{}", msg);
        warnings[hash] = true;
    }

    static void ClearWarnings()
    {
        get_warnings().clear();
    }

    template <typename... T>
    static void Info(fmt::format_string<T...> fmt, T&&... args)
    {
        std::string str = fmt::format(fmt, std::forward<T>(args)...);
        log_impl<LogInfo, ConsoleNormal>(str);
    }

    template <typename... T>
    static void Debug([[maybe_unused]] fmt::format_string<T...> fmt, [[maybe_unused]] T&&... args)
    {
#if ENABLE_DEBUG == 1
        std::string str = fmt::format(fmt, std::forward<T>(args)...);
        printf("%s\n", str.c_str());
#endif
    }

private:
    static inline std::string LogInfo()
    {
        return fmt::format(fg(fmt::color::green), "[INFO]");
    }

    static inline std::string LogWarning()
    {
        return fmt::format(fg(fmt::color::yellow), "[WARNING]");
    }

    static inline std::string LogFatal()
    {
        return fmt::format(fg(fmt::color::red), "[FATAL]");
    }

    static inline std::string LogDebug()
    {
        return fmt::format(fg(fmt::color::magenta), "[DEBUG]");
    }

    static inline void ConsoleNormal(std::string message)
    {
        fmt::print("{}", message);
    }

    template <auto PrefixFunction, auto OutputFunction>
    static inline void log_impl(std::string message)
    {
#if ENABLE_LOGGING == 1
        std::string prefix = PrefixFunction();
        if (prefix.empty())
        {
            return;
        }
        std::string messagef = fmt::format(" {}\n", message);
        OutputFunction(prefix + messagef);
#endif
    }

    static std::unordered_map<uint32_t, bool>& get_warnings()
    {
        static std::unordered_map<uint32_t, bool> warnings;
        return warnings;
    }
};
