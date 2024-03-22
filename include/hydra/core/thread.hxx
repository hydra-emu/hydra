#pragma once

#include <hydra/core.h>
#include <hydra/core/wrapper.hxx>
#include <memory>

namespace hydra::core::thread
{
    void start(std::shared_ptr<Wrapper> wrapper);
    void stop();
    void reset(HcResetType type);
    bool running();
} // namespace hydra::core::thread