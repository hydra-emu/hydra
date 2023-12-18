#pragma once

#include <fmt/format.h>
#include <string_view>

#if defined(HYDRA_WINDOWS)
#include <Windows.h>
#elif defined(HYDRA_ANDROID)
#include <android/log.h>
#else
#include <cstdio>
#endif

namespace hydra
{
    template <typename... T>
    void log(fmt::format_string<T...> fmt, T&&... args)
    {
        auto str = fmt::format(fmt, std::forward<T>(args)...);
#if defined(HYDRA_WINDOWS)
        OutputDebugStringA(str.c_str());
#elif defined(HYDRA_ANDROID)
        __android_log_print(ANDROID_LOG_INFO, "hydra", "%s", str.c_str());
#else
        printf("%s\n", str.c_str());
#endif
    }

    template <typename... T>
    void panic(fmt::format_string<T...> fmt, T&&... args)
    {
        log(fmt, std::forward<T>(args)...);
        std::abort();
    }
} // namespace hydra