#ifndef TKP_LOG_HXX
#define TKP_LOG_HXX
#include <fmt/core.h>
#include <fmt/color.h>
#include <include/global.hxx>

// #define ENABLE_LOGGING

struct Logger {
    static void Fatal(std::string message) {
        log_impl<LogFatal, ConsoleFatal>(message);
    }

    static void Warn(std::string message) {
        log_impl<LogWarning, ConsoleNormal>(message);
    }

    static void V(std::string message) {
        log_impl<LogVerbose1, ConsoleNormal>(message);
    }

    static void VV(std::string message) {
        log_impl<LogVerbose2, ConsoleNormal>(message);
    }

    static void VVV(std::string message) {
        log_impl<LogVerbose3, ConsoleNormal>(message);
    }

    static void VVVV(std::string message) {
        log_impl<LogVerbose4, ConsoleNormal>(message);
    }

    static void Info(std::string message) {
        log_impl<LogInfo, ConsoleNormal>(message);
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

    static inline std::string LogVerbose1() {
        if (Global::VerboseLevel < 1)
            return {};
        return fmt::format(fg(fmt::color::cyan), "[VERBOSE]");
    }

    static inline std::string LogVerbose2() {
        if (Global::VerboseLevel < 2)
            return {};
        return fmt::format(fg(fmt::color::cyan), "[VERBOSE]");
    }

    static inline std::string LogVerbose3() {
        if (Global::VerboseLevel < 3)
            return {};
        return fmt::format(fg(fmt::color::cyan), "[VERBOSE]");
    }

    static inline std::string LogVerbose4() {
        if (Global::VerboseLevel < 4)
            return {};
        return fmt::format(fg(fmt::color::cyan), "[VERBOSE]");
    }

    static inline std::string LogCustom(std::string scope) {
        return fmt::format(fg(fmt::color::magenta), "[{}]", scope);
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
        #ifdef ENABLE_LOGGING
        std::string prefix = PrefixFunction();
        if (prefix.empty())
            return;
        std::string messagef = fmt::format(" {}\n", message);
        OutputFunction(prefix + messagef);
        #endif
    }
};

#endif