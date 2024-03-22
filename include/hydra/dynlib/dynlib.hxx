#if defined(HYDRA_LIBDL)
#include <dlfcn.h>
#elif defined(HYDRA_WINDOWS)
#include <windows.h>
#endif

#include <filesystem>
#include <string>
#include <string_view>

namespace hydra::dynlib
{
    typedef void* handle_t;

    inline handle_t open(const std::filesystem::path& path)
    {
#if defined(HYDRA_LIBDL)
        return dlopen(path.string().c_str(), RTLD_LAZY);
#elif defined(HYDRA_WINDOWS)
        std::string spath = path.string();
        std::wstring wpath = std::wstring(spath.c_str(), spath.c_str() + spath.size());
        printf("Trying to convert string to wstring to load library with loadlibraryw, this is "
               "untested\n");
        return (void*)LoadLibraryW(wpath.c_str());
#else
#pragma message("dynlib_open not implemented for this platform")
#error dynlib_open not implemented for this platform
        return nullptr;
#endif
    }

    inline void* symbol(handle_t handle, const char* name)
    {
#if defined(HYDRA_LIBDL)
        return dlsym(handle, name);
#elif defined(HYDRA_WINDOWS)
        return (void*)GetProcAddress((HMODULE)handle, name);
#else
#pragma message("dynlib_get_symbol not implemented for this platform")
#error dynlib_get_symbol not implemented for this platform
        return nullptr;
#endif
    }

    inline void close(handle_t handle)
    {
#if defined(HYDRA_LIBDL)
        dlclose(handle);
#elif defined(HYDRA_WINDOWS)
        FreeLibrary((HMODULE)handle);
#else
#pragma message("dynlib_close not implemented for this platform")
#error dynlib_close not implemented for this platform
#endif
    }

    inline std::string_view extension()
    {
#if defined(HYDRA_LINUX) || defined(HYDRA_ANDROID) || defined(HYDRA_FREEBSD)
        return ".so";
#elif defined(HYDRA_WEB)
        return ".wasm";
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

    inline std::string error()
    {
#if defined(HYDRA_LIBDL)
        return dlerror();
#elif defined(HYDRA_WINDOWS)
        // DWORD error = GetLastError();
        // LPVOID buffer;
        // FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, error,
        // 0,
        //               (LPWSTR)&buffer, 0, NULL);
        // std::string ret = (char*)buffer;
        // LocalFree(buffer);
        // return ret;
        return ""; // TODO: fix
#endif
    }
} // namespace hydra::dynlib