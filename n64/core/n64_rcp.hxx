#pragma once

#include <array>
#include <cstdint>
#include <n64/core/n64_ai.hxx>
#include <n64/core/n64_rdp.hxx>
#include <n64/core/n64_rsp.hxx>
#include <n64/core/n64_vi.hxx>

namespace hydra::N64
{
    class N64;

    namespace Devices
    {
        class CPUBus;
        class CPU;
    } // namespace Devices
} // namespace hydra::N64
class N64Debugger;
class MmioViewer;

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
        friend class ::N64Debugger;
        friend class ::MmioViewer;
    };
} // namespace hydra::N64
