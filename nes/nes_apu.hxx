#pragma once

#include <cstdint>

namespace hydra::NES
{
    class CPUBus;

    class APU
    {
    public:
        void Tick();
        void Reset();

    private:
        void invalidate(uint8_t addr, uint8_t data);
        inline void tick_impl();
        bool should_tick_ = false;
        uint32_t clock_ = 0;
        uint16_t sq1_timer_ = 0;

        friend class CPUBus;
    };
} // namespace hydra::NES
