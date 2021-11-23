#include "gb_timer.h"
namespace TKPEmu::Gameboy::Devices {
    Timer::Timer(Bus* bus) : 
        bus_(bus), 
        DIV(bus->GetReference(addr_div)),
        TIMA(bus_->GetReference(addr_tim)),
        TMA(bus_->GetReference(addr_tma)),
        TAC(bus_->GetReference(addr_tac))
    {}
    void Timer::Reset() {
        DIV = 0;
        TAC = 0;
        TIMA = 0;
        TMA = 0;
        oscillator_ = 0;
        timer_counter_ = 0;
    }
    bool Timer::Update(uint8_t cycles) {
        oscillator_ += cycles;
        DIV += oscillator_ >> 8;
        oscillator_ &= 0xFF;
        if (TAC & 0b100 == 0) {
            return false;
        }
        timer_counter_ += cycles;
        auto interr_cycles = interr_times_[TAC & 0b11];
        if (timer_counter_ >= interr_cycles) {
            timer_counter_ -= interr_cycles;
            TIMA += 1;
            // If TIMA is 0 after incrementing, it means it has overflown
            if (TIMA == 0) {
                TIMA = TMA;
                return true;
            }
        }
        return false;
	}
}