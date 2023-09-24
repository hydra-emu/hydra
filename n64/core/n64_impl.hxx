#pragma once

#include <core/n64_cpu.hxx>
#include <core/n64_rcp.hxx>
#include <miniaudio.h>
#include <string>

namespace hydra::N64
{
    class N64
    {
    public:
        N64();
        bool LoadCartridge(std::string path);
        bool LoadIPL(std::string path);
        void RunFrame();
        void Reset();
        void SetMousePos(int32_t x, int32_t y);
        void SetAudioCallback(std::function<void(const std::vector<int16_t>&, int)> callback);

        void SetPollInputCallback(std::function<void()> callback)
        {
            cpu_.poll_input_callback_ = callback;
        }

        void SetReadInputCallback(std::function<int8_t(int, int, int)> callback)
        {
            cpu_.read_input_callback_ = callback;
        }

        int GetWidth()
        {
            return rcp_.vi_.width_;
        }

        int GetHeight()
        {
            return rcp_.vi_.height_;
        }

        void RenderVideo(std::vector<uint8_t>& data)
        {
            rcp_.vi_.Redraw(data);
        }

    private:
        RCP rcp_;
        CPUBus cpubus_;
        CPU cpu_;
    };
} // namespace hydra::N64
