#include <core/n64_rcp.hxx>

namespace hydra::N64
{
    void RCP::Reset()
    {
        // These default values are in little endian
        rsp_.Reset();
        rdp_.Reset();
        vi_.Reset();
    }

} // namespace hydra::N64