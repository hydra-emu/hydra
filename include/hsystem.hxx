#pragma once

#include <string>

#if defined(__x86_64__) || defined(__x86_64) || defined(__amd64) || defined(_M_X64)
#define HYDRA_X86_64
#elif defined(__i386__) || defined(__i386) || defined(_M_IX86)
#define HYDRA_X86
#elif defined(__aarch64__) || defined(__arm64__) || defined(_M_ARM64)
#define HYDRA_ARM64
#elif defined(__arm__)
#define HYDRA_ARM
#elif defined(__EMSCRIPTEN__)
#else
#pragma message("Unknown architecture")
#endif

inline std::string hydra_os()
{
    std::string ret;

#if defined(HYDRA_WINDOWS)
    ret = "Windows";
#elif defined(HYDRA_LINUX)
    ret = "Linux";
#elif defined(HYDRA_MACOS)
    ret = "macOS";
#elif defined(HYDRA_FREEBSD)
    ret = "FreeBSD";
#elif defined(HYDRA_WEB)
    return "Web WASM";
#elif defined(HYDRA_IOS)
    ret = "iOS";
#else
    ret = "Unknown";
#endif

#if defined(HYDRA_X86_64)
    ret += " x64";
#elif defined(HYDRA_X86)
    ret += " x86";
#elif defined(HYDRA_ARM64)
    ret += " arm64";
#elif defined(HYDRA_ARM)
    ret += " arm32";
#else
    ret += " Unknown";
#endif

    return ret;
}