#pragma once

#include <array>
#include <common/core_loader.hxx>
#include <core/core.h>
#include <filesystem>
#include <fmt/format.h>
#include <log.h>
#include <vector>

namespace hydra
{
    struct core_wrapper_t
    {
        core_wrapper_t(const std::filesystem::path& path)
        {
            dl_handle = dynlib_open(path.c_str());
            if (!dl_handle)
            {
                printf("Error while trying to load core: %s\n", path.c_str());
                log_fatal("Failed to dlopen core!");
                return;
            }
#define X(symbol)                                                                               \
    symbol##_p = reinterpret_cast<decltype(symbol##_p)>(dynlib_get_symbol(dl_handle, #symbol)); \
    if (!symbol##_p)                                                                            \
        log_fatal(fmt::format("No symbol {} in core {}", #symbol, path.string()).c_str());
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
