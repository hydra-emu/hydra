#ifndef TKP_LOG_HXX
#define TKP_LOG_HXX
#include <fmt/core.h>
#include <fmt/color.h>
#include <fmt/format.h>
#include <include/global.hxx>

#define ENABLE_LOGGING 0
#define ENABLE_DEBUG 0

struct Logger {
    template<typename... T>
    static void Fatal(fmt::format_string<T...> fmt, T&&... args) {
        std::string str = fmt::format(fmt, std::forward<T>(args)...);
        log_impl<LogFatal, ConsoleFatal>(str);
    }

    template<typename... T>
    static void Warn(fmt::format_string<T...> fmt, T&&... args) {
        std::string str = fmt::format(fmt, std::forward<T>(args)...);
        log_impl<LogWarning, ConsoleNormal>(str);
    }

    template<typename... T>
    static void Info(fmt::format_string<T...> fmt, T&&... args) {
        std::string str = fmt::format(fmt, std::forward<T>(args)...);
        log_impl<LogInfo, ConsoleNormal>(str);
    }

    template<typename... T>
    static void Debug(fmt::format_string<T...> fmt, T&&... args) {
        #if ENABLE_DEBUG == 1
        std::string str = fmt::format(fmt, std::forward<T>(args)...);
        log_impl<LogInfo, ConsoleNormal>(str);
        #endif
    }

private:
    static inline std::string LogInfo() {
        return fmt::format(fg(fmt::color::green), "[INFO]");
    }

    static inline std::string LogWarning() {
        return fmt::format(fg(fmt::color::yellow), "[WARNING]");
    }

    static inline std::string LogFatal() {
        return fmt::format(fg(fmt::color::red), "[FATAL]");
    }

    static inline std::string LogDebug() {
        return fmt::format(fg(fmt::color::magenta), "[DEBUG]");
    }

    static inline void ConsoleNormal(std::string message) {
        fmt::print("{}", message);
    }

    static inline void ConsoleFatal(std::string message) {
        ConsoleNormal(message);
        exit(1);
    }

    template <auto PrefixFunction, auto OutputFunction>
    static inline void log_impl(std::string message) {
        #if ENABLE_LOGGING == 1
        std::string prefix = PrefixFunction();
        if (prefix.empty())
            return;
        std::string messagef = fmt::format(" {}\n", message);
        OutputFunction(prefix + messagef);
        #endif
    }
};

#endif