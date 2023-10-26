#pragma once

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
#elif defined(__arm__) || defined(__aarch64__) || defined(__arm64__)
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
#define hydra_inline __always_inline
#elif defined(_MSC_VER)
#define hydra_inline __forceinline
#else
#define hydra_inline inline
#endif
