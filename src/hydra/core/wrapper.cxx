#include <hydra/common/log.hxx>
#include <hydra/core.h>
#include <hydra/core/wrapper.hxx>

extern "C" void* hydra_GetAddress(const char* name);

namespace hydra::core
{

    Wrapper::Wrapper(const std::filesystem::path& path) : handle(nullptr)
    {
        if (!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path))
        {
            hydra::panic("Core not found: {}", path.string());
            return;
        }

        if (path.extension() != hydra::dynlib::extension())
        {
            hydra::panic("Not a dynamic library: {}", path.string());
            return;
        }

        handle = hydra::dynlib::open(path);
        if (!handle)
        {
            hydra::panic("Failed to load core: {}\nError: {}", path.string(),
                         hydra::dynlib::error());
            return;
        }

        hcGetCoreInfo = (hcGetCoreInfoPtr)hydra::dynlib::symbol(handle, "hcGetCoreInfo");
        hcCreate = (hcCreatePtr)hydra::dynlib::symbol(handle, "hcCreate");
        hcDestroy = (hcDestroyPtr)hydra::dynlib::symbol(handle, "hcDestroy");
        hcReset = (hcResetPtr)hydra::dynlib::symbol(handle, "hcReset");
        hcSetRunState = (hcSetRunStatePtr)hydra::dynlib::symbol(handle, "hcSetRunState");
        hcLoadContent = (hcLoadContentPtr)hydra::dynlib::symbol(handle, "hcLoadContent");
        hcGetError = (hcGetErrorPtr)hydra::dynlib::symbol(handle, "hcGetError");

        if (!hcGetCoreInfo || !hcCreate || !hcDestroy || !hcReset || !hcSetRunState ||
            !hcLoadContent || !hcGetError)
        {
            hydra::log("Failed to load core functions: {}\nError: {}", path.string(),
                       hydra::dynlib::error());
            hydra::dynlib::close(handle);
            handle = nullptr;
        }

        void* (*hcInternalLoadFunctions)(void* (*loadFunctionPtr)(const char*)) =
            (decltype(hcInternalLoadFunctions))hydra::dynlib::symbol(handle,
                                                                     "hcInternalLoadFunctions");
        if (hcInternalLoadFunctions)
        {
            hcInternalLoadFunctions(hydra_GetAddress);
        }
        else
        {
            hydra::log("Failed to load internal functions: {}\nError: {}", path.string(),
                       hydra::dynlib::error());
            hydra::dynlib::close(handle);
            handle = nullptr;
        }
    }

    Wrapper::~Wrapper()
    {
        if (handle)
        {
            hydra::dynlib::close(handle);
        }
    }

    bool Wrapper::okay() const
    {
        return handle && hcGetCoreInfo && hcCreate && hcDestroy && hcReset && hcSetRunState &&
               hcLoadContent && hcGetError;
    }

} // namespace hydra::core