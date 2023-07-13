#include "nes_apu.hxx"

#define SQ1_VOL 0
#define SQ1_SWEEP 1
#define SQ1_LO 2
#define SQ1_HI 3

namespace hydra::NES {
    void APU::Tick() {
        if (should_tick_) {
            tick_impl();
        }
        should_tick_ ^= true;
    }
    
    void APU::tick_impl() {
        clock_ += 1;
    }

    void APU::invalidate(uint8_t addr, uint8_t data) {
        switch (addr) {
            case SQ1_VOL: {

                break;
            }
            case SQ1_SWEEP: {

                break;
            }
            case SQ1_LO: {
                sq1_timer_ &= ~0xFF;
                sq1_timer_ |= data;
                break;
            }
            case SQ1_HI: {
                uint16_t hi = data & 0b111;
                sq1_timer_ &= 0xFF;
                sq1_timer_ |= hi << 8;
                break;
            }
        }
    }
}