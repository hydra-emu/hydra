#pragma once

#include "n64_ai.hxx"
#include "n64_rdp.hxx"
#include "n64_rsp.hxx"
#include "n64_vi.hxx"
#include <array>
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
class N64Debugger;

namespace hydra::N64
{
    class RCP
    {
      public:
        void Reset();
        void SetPixelMode(uint8_t mode);
        void SetHVideo(uint32_t hvideo);
        void SetVVideo(uint32_t vvideo);
        void SetXScale(uint32_t xscale);
        void SetYScale(uint32_t yscale);
        bool Redraw();

      private:
        Vi vi_;
        Ai ai_;
        RSP rsp_;
        RDP rdp_;

        friend class hydra::N64::N64;
        friend class hydra::N64::CPUBus;
        friend class hydra::N64::CPU;
        friend class ::N64Debugger;
    };
} // namespace hydra::N64
