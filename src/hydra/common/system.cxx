#include <hydra/common/system.hxx>
#include <hydra/core.h>

namespace hydra::common
{

    HcArchitecture architecture()
    {
#if defined(__x86_64__) || defined(__x86_64) || defined(__amd64) || defined(_M_X64)
        return HC_ARCHITECTURE_X86_64;
#elif defined(__i386__) || defined(__i386) || defined(_M_IX86)
        return HC_ARCHITECTURE_X86;
#elif defined(__aarch64__) || defined(__arm64__) || defined(_M_ARM64)
        return HC_ARCHITECTURE_AARCH64;
#elif defined(__arm__)
        return HC_ARCHITECTURE_AARCH32;
#elif defined(__EMSCRIPTEN__)
        return HC_ARCHITECTURE_WASM;
#else
#pragma message("Unknown architecture")
        return HC_ARCHITECTURE_UNKNOWN;
#endif
    }

    HcOperatingSystem operatingSystem()
    {
#if defined(HYDRA_WINDOWS)
        return HC_OPERATING_SYSTEM_WINDOWS;
#elif defined(HYDRA_LINUX)
        return HC_OPERATING_SYSTEM_LINUX;
#elif defined(HYDRA_MACOS)
        return HC_OPERATING_SYSTEM_MACOS;
#elif defined(HYDRA_FREEBSD)
        return HC_OPERATING_SYSTEM_FREEBSD;
#elif defined(HYDRA_WEB)
        return HC_OPERATING_SYSTEM_WEB;
#elif defined(HYDRA_IOS)
        return HC_OPERATING_SYSTEM_IOS;
#elif defined(HYDRA_ANDROID)
        return HC_OPERATING_SYSTEM_ANDROID;
#else
        return HC_OPERATING_SYSTEM_UNKNOWN;
#endif
    }

} // namespace hydra::common