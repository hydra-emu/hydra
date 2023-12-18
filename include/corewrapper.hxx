#pragma once

#include <cstring>
#include <string>
#include <vector>
#if defined(HYDRA_LINUX) || defined(HYDRA_MACOS)
#include <dlfcn.h>
#elif defined(HYDRA_WINDOWS)
#include <windows.h>
#endif
#include "stb_image_write.h"
#include <filesystem>
#include <hydra/core.hxx>

namespace hydra
{

    typedef void* dynlib_handle_t;

    inline dynlib_handle_t dynlib_open(const char* path)
    {
#if defined(HYDRA_LIBDL)
        return dlopen(path, RTLD_LAZY | RTLD_GLOBAL);
#elif defined(HYDRA_WINDOWS)
        std::wstring wpath = std::wstring(path, path + std::strlen(path));
        printf("Trying to convert string to wstring to load library with loadlibraryw, this is "
               "untested\n");
        return (void*)LoadLibraryW(wpath.c_str());
#else
#pragma message("dynlib_open not implemented for this platform")
#error dynlib_open not implemented for this platform
        return nullptr;
#endif
    }

    inline void* dynlib_get_symbol(dynlib_handle_t handle, const char* name)
    {
#if defined(HYDRA_LIBDL)
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
#if defined(HYDRA_LIBDL)
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
#if defined(HYDRA_LINUX) || defined(HYDRA_ANDROID) || defined(HYDRA_FREEBSD)
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

    inline std::string dynlib_get_error()
    {
#if defined(HYDRA_LINUX) || defined(HYDRA_MACOS) || defined(HYDRA_ANDROID) || defined(HYDRA_FREEBSD)
        return dlerror();
#elif defined(HYDRA_WINDOWS)
        DWORD error = GetLastError();
        LPVOID buffer;
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, 0,
                      (LPWSTR)&buffer, 0, NULL);
        std::string ret = (char*)buffer;
        LocalFree(buffer);
        return ret;
#endif
    }

    struct CheatMetadata
    {
        bool enabled = false;
        std::string name{};
        std::string code{};
        uint32_t handle = hydra::BAD_CHEAT;
    };

    // Should only be made through the factory
    struct EmulatorWrapper
    {
        IBase* shell = nullptr;

        ~EmulatorWrapper();

        const char* GetInfo(hydra::InfoType type);
        const CheatMetadata& GetCheat(uint32_t handle);
        const std::vector<CheatMetadata>& GetCheats();

        bool LoadGame(const std::filesystem::path& path);
        uint32_t EditCheat(const CheatMetadata& cheat, uint32_t old_handle = hydra::BAD_CHEAT);
        void RemoveCheat(uint32_t handle);
        void EnableCheat(uint32_t handle);
        void DisableCheat(uint32_t handle);

        const std::vector<uint8_t>& GetIcon()
        {
            return icon_;
        }

    private:
        dynlib_handle_t handle;
        void (*destroy_function)(IBase*);
        const char* (*get_info_function)(hydra::InfoType);
        std::string game_hash_;

        EmulatorWrapper(IBase* shl, dynlib_handle_t hdl, void (*dfunc)(IBase*),
                        const char* (*gfunc)(hydra::InfoType));

        void init_cheats();
        void save_cheats();

        std::vector<CheatMetadata> cheats_;
        std::vector<uint8_t> icon_;

        EmulatorWrapper(const EmulatorWrapper&) = delete;
        friend struct EmulatorFactory;
    };

    struct EmulatorFactory
    {
        static std::shared_ptr<EmulatorWrapper> Create(const std::string& path)
        {
            dynlib_handle_t handle = dynlib_open(path.c_str());

            if (!handle)
            {
                printf("Failed to load library %s: %s\n", path.c_str(), dynlib_get_error().c_str());
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

            auto emulator = std::shared_ptr<EmulatorWrapper>(
                new EmulatorWrapper(create_emu_p(), handle, destroy_emu_p, get_info_p));

            if (get_info_p(hydra::InfoType::IconData) != nullptr &&
                get_info_p(hydra::InfoType::IconWidth) != nullptr &&
                get_info_p(hydra::InfoType::IconHeight) != nullptr)
            {
                int width = std::atoi(get_info_p(hydra::InfoType::IconWidth));
                int height = std::atoi(get_info_p(hydra::InfoType::IconHeight));

                if (width <= 0 || height <= 0)
                {
                    printf("Invalid icon size %dx%d\n", width, height);
                    return nullptr;
                }

                std::vector<uint8_t> temp = std::vector<uint8_t>(width * height * 4);
                memcpy(temp.data(), get_info_p(hydra::InfoType::IconData), temp.size());

                for (size_t i = 0; i < temp.size(); i += 4)
                {
                    uint8_t r = temp[i + 0];
                    uint8_t g = temp[i + 1];
                    uint8_t b = temp[i + 2];
                    uint8_t a = temp[i + 3];
                    temp[i + 0] = a;
                    temp[i + 1] = b;
                    temp[i + 2] = g;
                    temp[i + 3] = r;
                }

                int size;
                void* data = stbi_write_png_to_mem(temp.data(), 0, width, height, 4, &size);
                emulator->icon_.resize(size);
                memcpy(emulator->icon_.data(), data, size);
                free(data);
            }

            return emulator;
        }

        static std::shared_ptr<EmulatorWrapper> Create(const std::filesystem::path& path)
        {
            return Create(path.string());
        }
    };

} // namespace hydra
