#include <chrono>
#include <iostream>
#include <n64/core/n64_impl.hxx>

namespace hydra::N64
{
    N64::N64() : cpubus_(rcp_), cpu_(cpubus_, rcp_)
    {
        Reset();
    }

    bool N64::LoadCartridge(std::string path)
    {
        return cpu_.cpubus_.LoadCartridge(path);
    }

    bool N64::LoadIPL(std::string path)
    {
        if (!path.empty())
        {
            return cpu_.cpubus_.LoadIPL(path);
        }
        return false;
    }

    void N64::RunFrame()
    {
        static int cycles = 0;
        for (int f = 0; f < 1; f++)
        { // fields
            for (int hl = 0; hl < cpu_.rcp_.vi_.num_halflines_; hl++)
            { // halflines
                cpu_.rcp_.vi_.vi_v_current_ = (hl << 1) + f;
                cpu_.check_vi_interrupt();
                while (cycles <= cpu_.rcp_.vi_.cycles_per_halfline_)
                {
                    static int cpu_cycles = 0;
                    cpu_cycles++;
                    cpu_.Tick();
                    rcp_.ai_.Step();
                    if (!cpu_.rcp_.rsp_.IsHalted())
                    {
                        while (cpu_cycles > 2)
                        {
                            cpu_.rcp_.rsp_.Tick();
                            if (!cpu_.rcp_.rsp_.IsHalted())
                            {
                                cpu_.rcp_.rsp_.Tick();
                            }
                            cpu_cycles -= 3;
                        }
                    }
                    else
                    {
                        cpu_cycles = 0;
                    }
                    cycles++;
                }
                cycles -= cpu_.rcp_.vi_.cycles_per_halfline_;
            }
            cpu_.check_vi_interrupt();
        }
        if (std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now() - cpu_.last_second_time_)
                .count() >= 1000)
        {
            cpu_.last_second_time_ = std::chrono::high_resolution_clock::now();
            cpu_.vis_per_second_ = rcp_.vi_.vis_counter_;
            // printf("VIs: %d\n", cpu_.vis_per_second_);
            rcp_.vi_.vis_counter_ = 0;
        }
    }

    void N64::Reset()
    {
        cpu_.Reset();
        rcp_.Reset();
    }

    void N64::SetMousePos(int32_t x, int32_t y)
    {
        cpu_.mouse_delta_x_ = x - cpu_.mouse_x_;
        cpu_.mouse_delta_y_ = y - cpu_.mouse_y_;
        cpu_.mouse_x_ = x;
        cpu_.mouse_y_ = y;
    }
} // namespace hydra::N64