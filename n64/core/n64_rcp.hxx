#pragma once

#include <array>
#include <core/n64_ai.hxx>
#include <core/n64_rdp.hxx>
#include <core/n64_rsp.hxx>
#include <core/n64_vi.hxx>
#include <cstdint>

namespace hydra::N64
{
    class N64;

    namespace Devices
    {
        class CPUBus;
        class CPU;
    } // namespace Devices
} // namespace hydra::N64

namespace hydra::N64
{
    class RCP final
    {
    public:
        void Reset();

    private:
        Vi vi_;
        Ai ai_;
        RSP rsp_;
        RDP rdp_;

        friend class hydra::N64::N64;
        friend class hydra::N64::CPUBus;
        friend class hydra::N64::CPU;
    };
} // namespace hydra::N64
