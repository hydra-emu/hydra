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
#include <core/core.hxx>
#include <filesystem>

namespace hydra
{

    typedef void* dynlib_handle_t;

    inline dynlib_handle_t dynlib_open(const char* path)
    {
#if defined(HYDRA_LINUX) || defined(HYDRA_MACOS)
        return dlopen(path, RTLD_LAZY | RTLD_GLOBAL);
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

    // Should only be made through the factory
    struct EmulatorWrapper
    {
        BaseEmulatorInterface* shell = nullptr;

        ~EmulatorWrapper()
        {
            destroy_function(shell);
            dynlib_close(handle);
        }

    private:
        dynlib_handle_t handle;
        void (*destroy_function)(BaseEmulatorInterface*);

        EmulatorWrapper(BaseEmulatorInterface* shell, dynlib_handle_t handle,
                        void (*destroy_function)(BaseEmulatorInterface*))
            : shell(shell), handle(handle), destroy_function(destroy_function)
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

            return std::unique_ptr<EmulatorWrapper>(
                new EmulatorWrapper(create_emu_p(), handle, destroy_emu_p));
        }

        static std::unique_ptr<EmulatorWrapper> Create(const std::filesystem::path& path)
        {
            return Create(path.string());
        }
    };

} // namespace hydra