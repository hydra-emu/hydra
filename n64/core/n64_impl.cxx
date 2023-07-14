#include "n64_impl.hxx"
#include <iostream>

namespace hydra::N64
{
    N64::N64(bool& should_draw) : cpubus_(rcp_), cpu_(cpubus_, rcp_, should_draw) { Reset(); }

    bool N64::LoadCartridge(std::string path) { return cpu_.cpubus_.LoadCartridge(path); }

    bool N64::LoadIPL(std::string path)
    {
        if (!path.empty())
        {
            return cpu_.cpubus_.LoadIPL(path);
        }
        return false;
    }

    void N64::Update()
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
                    } else
                    {
                        cpu_cycles = 0;
                    }
                    cycles++;
                }
                cycles -= cpu_.rcp_.vi_.cycles_per_halfline_;
            }
            cpu_.check_vi_interrupt();
        }
        cpu_.should_draw_ = rcp_.Redraw();
    }

    void N64::Reset()
    {
        cpu_.Reset();
        rcp_.Reset();
    }
} // namespace hydra::N64