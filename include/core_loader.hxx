#pragma once

#include "hsystem.h"
#include <string>
#if defined(HYDRA_LINUX) || defined(HYDRA_MACOS)
#include <dlfcn.h>
#elif defined(HYDRA_WINDOWS)
#include <windows.h>
#elif defined(HYDRA_WII)
#include "ELFIO/elfio/elfio.hpp"
#endif
#include <core/core.h>
#include <filesystem>

namespace hydra
{

    typedef void* dynlib_handle_t;

    inline dynlib_handle_t dynlib_open(const char* path)
    {
#if defined(HYDRA_LINUX) || defined(HYDRA_MACOS)
        return dlopen(path, RTLD_LAZY);
#elif defined(HYDRA_WINDOWS)
        std::wstring wpath = std::wstring(path.begin(), path.end());
        printf("Trying to convert string to wstring to load library with loadlibraryw, this is "
               "untested\n",
               path, wpath.c_str());
        return (void*)LoadLibraryW(wpath.c_str());
#elif defined(HYDRA_WII)
        ELFIO::elfio* reader = new ELFIO::elfio();
        if (!reader->load(path))
        {
            printf("Failed to load library %s\n", path);
            return nullptr;
        }
        if (reader->get_class() != ELFCLASS32)
        {
            printf("Library %s is not 32-bit\n", path);
            return nullptr;
        }
        if (reader->get_encoding() != ELFDATA2MSB)
        {
            printf("Library %s is not big endian\n", path);
            return nullptr;
        }
        return reader;
#else
#pragma message("dynlib_open not implemented for this platform")
#error dynlib_open not implemented for this platform
        return nullptr;
#endif
    }

    inline void* dynlib_get_symbol(dynlib_handle_t handle, const char* name)
    {
#if defined(HYDRA_LINUX) || defined(HYDRA_MACOS)
        return dlsym(handle, name);
#elif defined(HYDRA_WINDOWS)
        return (void*)GetProcAddress((HMODULE)handle, name);
#elif defined(HYDRA_WII)
        ELFIO::elfio* reader = (ELFIO::elfio*)handle;
#else
#pragma message("dynlib_get_symbol not implemented for this platform")
#error dynlib_get_symbol not implemented for this platform
        return nullptr;
#endif
    }

    inline void dynlib_close(dynlib_handle_t handle)
    {
#if defined(HYDRA_LINUX) || defined(HYDRA_MACOS)
        dlclose(handle);
#elif defined(HYDRA_WINDOWS)
        FreeLibrary((HMODULE)handle);
#elif defined(HYDRA_WII)
        delete (ELFIO::elfio*)handle;
#else
#pragma message("dynlib_close not implemented for this platform")
#error dynlib_close not implemented for this platform
#endif
    }

    inline std::string dynlib_get_extension()
    {
#if defined(HYDRA_LINUX)
        return ".so";
#elif defined(HYDRA_MACOS)
        return ".dylib";
#elif defined(HYDRA_WINDOWS)
        return ".dll";
#else
#pragma message("dynlib_get_extension not implemented for this platform")
#error dynlib_get_extension not implemented for this platform
        return "";
#endif
    }

    struct core_wrapper_t
    {
        core_wrapper_t(const std::filesystem::path& path)
        {
            dl_handle = dynlib_open(path.c_str());
            if (!dl_handle)
            {
                printf("Error while trying to load core: %s\n", path.c_str());
                return;
            }
#define X(symbol)                                                                               \
    symbol##_p = reinterpret_cast<decltype(symbol##_p)>(dynlib_get_symbol(dl_handle, #symbol)); \
    if (!symbol##_p)                                                                            \
        printf("Error while trying to load symbol: %s\n", #symbol);
            HC_SYMBOLS
#undef X
        }

        ~core_wrapper_t()
        {
            if (dl_handle)
            {
                dynlib_close(dl_handle);
            }
        }

#define X(name) decltype(name)* name##_p;
        HC_SYMBOLS
#undef X

        void* core_handle = nullptr;
        void* dl_handle;
    };

} // namespace hydra