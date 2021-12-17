#include "gb_timer.h"
namespace TKPEmu::Gameboy::Devices {
    Timer::Timer(Bus* bus) : 
        bus_(bus), 
        DIV(bus->GetReference(addr_div)),
        TIMA(bus_->GetReference(addr_tim)),
        TAC(bus_->GetReference(addr_tac)),
        TMA(bus_->GetReference(addr_tma)),
        IF(bus_->GetReference(addr_ifl))
    {}
    void Timer::Reset() {
        DIV = 0;
        TAC = 0;
        TIMA = 0;
        TMA = 0;
        oscillator_ = 0;
        timer_counter_ = 0;
		div_reset_index_ = 0;
		tima_overflow_ = false;
    }
    bool Timer::Update(uint8_t cycles, uint8_t old_if) {
		bool enabled = TAC & 0b100;
        if (tima_overflow_) {
			// TIMA might've changed in this strange cycle (see the comment below)
			// If it changes in that cycle, it doesn't update to be equal to TMA
			if (TIMA == 0) {
				TIMA = TMA;
				// If this isn't true, IF has changed during this instruction so the new value persists
				if (IF == old_if) {
					IF |= IFInterrupt::TIMER;
				}
			}
			tima_overflow_ = false;
		}
        int freq = interr_times_[TAC & 0b11];
		if (bus_->DIVReset) {
			bus_->DIVReset = false;
			if (div_reset_index_ >= freq / 2) {
				TIMA++;
			}
			oscillator_ = 0;
			timer_counter_ = 0;
			div_reset_index_ = -1;
		}
		oscillator_ += cycles;
		// Divider always equals the top 8 bits of the oscillator
		DIV = oscillator_ >> 8;
		if (div_reset_index_ != -1)
			div_reset_index_ += cycles;
		if (div_reset_index_ > freq) {
			TIMA++;
			div_reset_index_ = -1;
		}
		if (enabled) {
			timer_counter_ += cycles;
			while (timer_counter_ >= freq) {
				timer_counter_ -= freq;
				//timer_counter_ = get_clk_freq();
				if (TIMA == 0xFF) {
					// After TIMA overflows, it stays 00 for 1 clock and *then* becomes =TMA
					TIMA = 0;
					tima_overflow_ = true;
					return true;
				}
				else {
					TIMA++;
				}
			}
		}
		return false;
	}
}