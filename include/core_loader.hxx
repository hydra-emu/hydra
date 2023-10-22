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
#include <filesystem>
#include <hydra/core.hxx>

namespace hydra
{

    typedef void* dynlib_handle_t;

    inline dynlib_handle_t dynlib_open(const char* path)
    {
#if defined(HYDRA_LINUX) || defined(HYDRA_MACOS) || defined(HYDRA_ANDROID)
        return dlopen(path, RTLD_NOW | RTLD_GLOBAL);
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
#if defined(HYDRA_LINUX) || defined(HYDRA_MACOS) || defined(HYDRA_ANDROID)
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
#if defined(HYDRA_LINUX) || defined(HYDRA_MACOS) || defined(HYDRA_ANDROID)
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
#if defined(HYDRA_LINUX) || defined(HYDRA_ANDROID)
        return ".so";
#elif defined(HYDRA_MACOS)
        return ".dylib";
#elif defined(HYDRA_WINDOWS)
        return ".dll";
#elif defined(HYDRA_WII)
        return ".elf";
#else
#pragma message("dynlib_get_extension not implemented for this platform")
#error dynlib_get_extension not implemented for this platform
        return "";
#endif
    }

    // Should only be made through the factory
    struct EmulatorWrapper
    {
        IBase* shell = nullptr;

        ~EmulatorWrapper()
        {
            destroy_function(shell);
            dynlib_close(handle);
        }

        const char* GetInfo(hydra::InfoType type)
        {
            return get_info_function(type);
        }

    private:
        dynlib_handle_t handle;
        void (*destroy_function)(IBase*);
        const char* (*get_info_function)(hydra::InfoType);

        EmulatorWrapper(IBase* shl, dynlib_handle_t hdl, void (*dfunc)(IBase*),
                        const char* (*gfunc)(hydra::InfoType))
            : shell(shl), handle(hdl), destroy_function(dfunc), get_info_function(gfunc)
        {
        }

        EmulatorWrapper(const EmulatorWrapper&) = delete;
        friend struct EmulatorFactory;
    };

    struct EmulatorFactory
    {
        static std::unique_ptr<EmulatorWrapper> Create(const std::string& path)
        {
            dynlib_handle_t handle = dynlib_open(path.c_str());

            if (!handle)
            {
                printf("Failed to load library %s\n", path.c_str());
                return nullptr;
            }
            auto create_emu_p =
                (decltype(hydra::createEmulator)*)dynlib_get_symbol(handle, "createEmulator");
            if (!create_emu_p)
            {
                printf("Failed to find createEmulator in %s\n", path.c_str());
                dynlib_close(handle);
                return nullptr;
            }

            auto destroy_emu_p =
                (decltype(hydra::destroyEmulator)*)dynlib_get_symbol(handle, "destroyEmulator");

            if (!destroy_emu_p)
            {
                printf("Failed to find destroyEmulator in %s\n", path.c_str());
                dynlib_close(handle);
                return nullptr;
            }

            auto get_info_p = (decltype(hydra::getInfo)*)dynlib_get_symbol(handle, "getInfo");

            if (!get_info_p)
            {
                printf("Failed to find getInfo in %s\n", path.c_str());
                dynlib_close(handle);
                return nullptr;
            }

            return std::unique_ptr<EmulatorWrapper>(
                new EmulatorWrapper(create_emu_p(), handle, destroy_emu_p, get_info_p));
        }

        static std::unique_ptr<EmulatorWrapper> Create(const std::filesystem::path& path)
        {
            return Create(path.string());
        }
    };

} // namespace hydra