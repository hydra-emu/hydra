#pragma once

#include <n64/core/n64_cpu.hxx>
#include <n64/core/n64_rcp.hxx>
#include <string>

class MmioViewer;

namespace hydra::N64
{
    class N64
    {
    public:
        N64();
        bool LoadCartridge(std::string path);
        bool LoadIPL(std::string path);
        void Update();
        void Reset();
        void SetMousePos(int32_t x, int32_t y);

        void* GetColorData()
        {
            return rcp_.vi_.framebuffer_ptr_;
        }

        int GetWidth()
        {
            return rcp_.vi_.width_;
        }

        int GetHeight()
        {
            return rcp_.vi_.height_;
        }

        void SetKeyState(uint32_t key, bool state)
        {
            cpu_.key_state_[key] = state;
        }

    private:
        RCP rcp_;
        CPUBus cpubus_;
        CPU cpu_;
        friend class N64_TKPWrapper;
        friend class ::N64Debugger;
        friend class ::MmioViewer;
    };
} // namespace hydra::N64
