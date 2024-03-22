#pragma once

// Sanity checks to verify at compile time that definitions passed by CMake are correct

#ifdef HYDRA_WEB
#if !defined(EMSCRIPTEN) && !defined(__EMSCRIPTEN__)
#error "HYDRA_WEB defined but EMSCRIPTEN not defined"
#endif
#endif

#ifdef HYDRA_WINDOWS
#if !defined(_WIN32) && !defined(_WIN64)
#error "HYDRA_WINDOWS defined but _WIN32 or _WIN64 not defined"
#endif
#endif

#ifdef HYDRA_LINUX
#if !defined(__linux__)
#error "HYDRA_LINUX defined but __linux__ not defined"
#endif
#if defined(__ANDROID__)
#error "HYDRA_LINUX defined but __ANDROID__ defined"
#endif
#if defined(__FreeBSD__)
#error "HYDRA_LINUX defined but __FreeBSD__ defined"
#endif
#endif

#ifdef HYDRA_FREEBSD
#if !defined(__FreeBSD__)
#error "HYDRA_FREEBSD defined but __FreeBSD__ not defined"
#endif
#endif

#if defined(HYDRA_MACOS) || defined(HYDRA_IOS)
#if !defined(__APPLE__)
#error "HYDRA_MACOS defined but __APPLE__ not defined"
#endif
#endif

#ifdef HYDRA_ANDROID
#if !defined(__ANDROID__)
#error "HYDRA_ANDROID defined but __ANDROID__ not defined"
#endif
#endif
