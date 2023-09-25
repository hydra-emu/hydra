#pragma once

#include <array>
#include <common/core.h>
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
            dlhandle = dlopen(path.c_str(), RTLD_LAZY);
            if (!dlhandle)
            {
                log_fatal("Failed to dlopen core!");
                return;
            }
        }

        ~core_wrapper_t()
        {
            if (dlhandle)
            {
                dlclose(dlhandle);
            }
        }

#define X(name) decltype(name)* name;
        HYDRA_CORE_SYMBOLS
#undef X

        void* dlhandle;
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
