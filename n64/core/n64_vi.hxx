#pragma once

#include <cstdint>
#include <vector>

namespace hydra::N64
{
    class N64;
}

namespace hydra::N64
{
    class RCP;
    class CPU;
    class CPUBus;
    union MIInterrupt;

    struct Vi
    {
        void Reset();
        bool Redraw();
        uint8_t* GetFramebufferPtr();
        int GetWidth();
        int GetHeight();
        uint32_t ReadWord(uint32_t addr);
        void WriteWord(uint32_t addr, uint32_t data);

        void InstallBuses(uint8_t* rdram_ptr)
        {
            rdram_ptr_ = rdram_ptr;
        }

        void SetMIPtr(MIInterrupt* mi_interrupt)
        {
            mi_interrupt_ = mi_interrupt;
        }

    private:
        uint32_t vi_ctrl_ = 0;
        uint32_t vi_origin_ = 0;
        uint32_t vi_width_ = 0;
        uint32_t vi_v_intr_ = 0x3ff;
        uint32_t vi_v_current_ = 0;
        uint32_t vi_burst_ = 0;
        uint32_t vi_v_sync_ = 0;
        uint32_t vi_h_sync_ = 0;
        uint32_t vi_h_sync_leap_ = 0;
        uint32_t vi_h_start_ = 0;
        uint32_t vi_h_end_ = 0;
        uint32_t vi_v_start_ = 0;
        uint32_t vi_v_end_ = 0;
        uint32_t vi_v_burst_ = 0;
        uint32_t vi_x_scale_ = 0;
        uint32_t vi_y_scale_ = 0;
        uint32_t vi_test_addr_ = 0;
        uint32_t vi_staged_data_ = 0;
        int vis_counter_ = 0;
        int width_ = 320, height_ = 240;
        int num_halflines_ = 262;
        int cycles_per_halfline_ = 1000;

        uint8_t pixel_mode_ = 0;
        std::vector<uint8_t> framebuffer_;
        uint8_t* framebuffer_ptr_ = nullptr;
        uint8_t* memory_ptr_ = nullptr;
        uint8_t* rdram_ptr_ = nullptr;
        MIInterrupt* mi_interrupt_ = nullptr;

        inline void set_pixel(int x, int y, uint32_t color);
        friend class hydra::N64::RCP;
        friend class hydra::N64::CPU;
        friend class hydra::N64::CPUBus;
        friend class hydra::N64::N64;
    };
} // namespace hydra::N64