#include <hydra/common/version.hxx>

namespace hydra::common
{
    const std::string& version()
    {
        static const std::string version = "hydra 0.1.0";
        return version;
    }
} // namespace hydra::common