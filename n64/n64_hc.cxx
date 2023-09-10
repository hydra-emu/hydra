#include <n64/n64_hc.hxx>

namespace hydra
{

    HydraCore_N64::HydraCore_N64() {}

    HydraCore_N64::~HydraCore_N64() {}

    bool HydraCore_N64::load_file(const std::string& type, const std::string& path)
    {
        if (type == "ipl")
        {
            return impl_.LoadIPL(path);
        }
        else if (type == "rom")
        {
            if (!ipl_loaded_)
                return false;
            return impl_.LoadCartridge(path);
        }
        return false;
    }

} // namespace hydra