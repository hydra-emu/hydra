#include <common/str_hash.hxx>
#include <error_factory.hxx>
#include <filesystem>
#include <iostream>
#include <memory>
#include <settings.hxx>
#include <ui_common.hxx>

namespace hydra
{
    std::string UiCommon::GetSavePath()
    {
        static std::string dir;
        if (dir.empty())
        {
#if defined(__linux__)
            dir = getenv("HOME") + std::string("/.config/hydra/");
#elif defined(_WIN32)
            dir = getenv("APPDATA") + std::string("/hydra/");
#elif defined(__APPLE__)
            dir = getenv("HOME") + std::string("/Library/Application Support/hydra/");
#endif
            if (dir.empty())
            {
                throw ErrorFactory::generate_exception(
                    __func__, __LINE__, "GetSavePath was not defined for this environment");
            }
            if (!std::filesystem::create_directories(dir))
            {
                if (std::filesystem::exists(dir))
                {
                    return dir;
                }
                throw ErrorFactory::generate_exception(__func__, __LINE__,
                                                       "Failed to create directories");
            }
        }
        return dir;
    }

    std::unique_ptr<core_wrapper_t> UiCommon::Create(const std::filesystem::path& path)
    {
        auto core = std::make_unique<core_wrapper_t>(path);
#define X(symbol)                                                                            \
    core->symbol = reinterpret_cast<decltype(core->symbol)>(dlsym(core->dlhandle, #symbol)); \
    if (!core->symbol)                                                                       \
    {                                                                                        \
        std::cerr << "Failed to load " << #symbol << ": " << dlerror() << std::endl;         \
        return nullptr;                                                                      \
    }
        HYDRA_CORE_SYMBOLS
#undef X
        return core;
    }

} // namespace hydra