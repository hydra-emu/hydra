#include <atomic>
#include <chrono>
#include <filesystem>
#include <gb/gb_hc.hxx>
#include <iostream>

namespace hydra
{
    HydraCore_Gameboy::HydraCore_Gameboy()
        : channel_array_ptr_(std::make_shared<ChannelArray>()), bus_(channel_array_ptr_),
          apu_(channel_array_ptr_, bus_.GetReference(addr_NR52)), ppu_(bus_),
          timer_(channel_array_ptr_, bus_), cpu_(bus_, ppu_, apu_, timer_),
          joypad_(bus_.GetReference(addr_joy)), interrupt_flag_(bus_.GetReference(addr_if))
    {
        (*channel_array_ptr_.get())[0].HasSweep = true;
        apu_.UseSound = true;
        apu_.InitSound();
    }

    void HydraCore_Gameboy::Reset()
    {
        bus_.SoftReset();
        cpu_.Reset(true);
        timer_.Reset();
        ppu_.Reset();
    }

    void HydraCore_Gameboy::run_frame()
    {
        for (int i = 0; i < 70224; i++)
        {
            uint8_t old_if = interrupt_flag_;
            int clk = 0;
            if (!cpu_.skip_next_)
            {
                clk = cpu_.Update();
            }
            cpu_.skip_next_ = false;
            if (timer_.Update(clk, old_if))
            {
                if (cpu_.halt_)
                {
                    cpu_.halt_ = false;
                    cpu_.skip_next_ = true;
                }
            }
            ppu_.Update(clk);
            apu_.Update(clk);
        }
    }

    bool HydraCore_Gameboy::load_file(const std::string& type, const std::string& path)
    {
        if (type == "rom")
        {
            auto loaded = bus_.LoadCartridge(path);
            ppu_.UseCGB = bus_.UseCGB;
            return loaded;
        }
        return false;
    }

    VideoInfo HydraCore_Gameboy::render_frame()
    {
        VideoInfo info;
        info.width = 160;
        info.height = 144;
        info.data.resize(info.width * info.height * 4);
        memcpy(info.data.data(), ppu_.GetScreenData(), info.data.size());
        info.format = VideoFormat::RGBA8888;
        return info;
    }

    AudioInfo HydraCore_Gameboy::render_audio()
    {
        AudioInfo info;
        return info;
    }

} // namespace hydra
