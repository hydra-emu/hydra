#include <core.hxx>

namespace hydra
{

    std::future<void> Core::RunFrameAsync()
    {
        return std::async(std::launch::async, [this]() { run_frame(); });
    }

} // namespace hydra