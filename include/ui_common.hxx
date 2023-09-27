#pragma once

#include <array>
#include <core/core.h>
#include <dlfcn.h>
#include <filesystem>
#include <log.h>
#include <vector>

namespace hydra
{
    struct core_wrapper_t
    {
        core_wrapper_t(const std::filesystem::path& path)
        {
            dl_handle = dlopen(path.c_str(), RTLD_LAZY);
            if (!dl_handle)
            {
                printf("Error while trying to load core: %s, %s\n", path.c_str(), dlerror());
                log_fatal("Failed to dlopen core!");
                return;
            }
        }

        ~core_wrapper_t()
        {
            if (dl_handle)
            {
                dlclose(dl_handle);
            }
        }

#define X(name) decltype(name)* name;
        HC_SYMBOLS
#undef X

        void* core_handle = nullptr;
        void* dl_handle;
    };

    struct emulator_data_t
    {
        std::string Name;
        std::vector<std::string> Extensions;
    };

    class UiCommon
    {
    public:
        static std::string GetSavePath();
        static std::unique_ptr<core_wrapper_t> Create(const std::filesystem::path& path);
    };
} // namespace hydra
