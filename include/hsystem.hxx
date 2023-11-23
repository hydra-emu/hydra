#pragma once

#include <string>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
#define HYDRA_WINDOWS
#elif defined(__linux__)
#define HYDRA_LINUX
#elif defined(__APPLE__)
#define HYDRA_MACOS
#elif defined(__FreeBSD__)
#define HYDRA_FREEBSD
#elif defined(__unix__) || defined(__unix) || defined(unix)
#define HYDRA_UNIX
#elif defined(__ANDROID__)
#define HYDRA_ANDROID
#elif defined(GEKKO)
#define HYDRA_WII
#else
#pragma message("Unknown platform")
#endif

#if defined(__x86_64__) || defined(__x86_64) || defined(__amd64) || defined(_M_X64)
#define HYDRA_X86_64
#elif defined(__i386__) || defined(__i386) || defined(_M_IX86)
#define HYDRA_X86
#elif defined(__aarch64__) || defined(__arm64__) || defined(_M_ARM64)
#define HYDRA_ARM64
#elif defined(__arm__)
#define HYDRA_ARM
#else
#pragma message("Unknown architecture")
#endif

#ifdef HYDRA_WINDOWS
#include <immintrin.h>
#include <intrin.h>
#pragma intrinsic(_mul128)
#else
#ifdef HYDRA_X86_64
#include <x86intrin.h>
#elif defined(HYDRA_ARM)
#include <arm_acle.h>
#endif
#endif

#ifdef __clang__
#ifdef HYDRA_MACOS
#define hydra_inline inline
#else
#define hydra_inline [[clang::always_inline]] inline
#endif
#elif defined(__GNUC__)
#define hydra_inline __always_inline inline
#elif defined(_MSC_VER)
#define hydra_inline __forceinline inline
#else
#define hydra_inline inline
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
#else
    ret = "Unknown";
#endif

#if defined(HYDRA_X86_64)
    ret += " x86_64";
#elif defined(HYDRA_X86)
    ret += " x86";
#elif defined(HYDRA_ARM64)
    ret += " ARM64";
#elif defined(HYDRA_ARM)
    ret += " ARM";
#else
    ret += " Unknown";
#endif

    return ret;
}