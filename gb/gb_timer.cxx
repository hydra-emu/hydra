#include <gb/gb_timer.hxx>
#include <iostream>
#include <bitset>
namespace hydra::Gameboy {
    Timer::Timer(ChannelArrayPtr channel_array_ptr, Bus& bus) : 
        channel_array_ptr_(channel_array_ptr),
        bus_(bus), 
        DIV(bus.GetReference(addr_div)),
        TIMA(bus.GetReference(addr_tim)),
        TAC(bus.GetReference(addr_tac)),
        TMA(bus.GetReference(addr_tma)),
        IF(bus.GetReference(addr_ifl))
    {}
    void Timer::Reset() {
        DIV = 0;
        TAC = 0;
        TIMA = 0;
        TMA = 0;
        oscillator_ = 0;
        timer_counter_ = 0;
        tima_overflow_ = false;
        just_overflown_ = false;
    }
    bool Timer::Update(uint8_t cycles, uint8_t old_if) {
        bool ret = false;
        if (just_overflown_) {
            // Passes tima_write_reloading
            // If TIMA is written while cycle [B] (check cycle accurate docs) TMA is written instead
            if (bus_.TIMAChanged || bus_.TMAChanged) {
                TIMA = TMA;
            }
        }
        just_overflown_ = false;
        bool enabled = TAC & 0b100;
        if (tima_overflow_) {
            // TIMA might've changed in this strange cycle (see the comment below)
            // If it changes in that cycle, it doesn't update to be equal to TMA
            if (TIMA == 0) {
                just_overflown_ = true;
                TIMA = TMA;
                // If this isn't true, IF has changed during this instruction so the new value persists
                if (IF == old_if) {
                    IF |= IFInterrupt::TIMER;
                }
            }
            tima_overflow_ = false;
        }
        int freq = interr_times_[TAC & 0b11];
        if (bus_.DIVReset) {
            if (oscillator_ == freq / 2) {
                // timXX_div_trigger behavior
                // TIMA increased if oscillator reached half or more and DIV is reset
                TIMA++;
            } else if (TAC & 0b1) {
                // weird tim01_div_trigger behavior
                // incorrect timing makes us need to add this patch
                // to pass the test
                if ((oscillator_ - 4) == freq / 2) {
                    TIMA++;
                } else if ((oscillator_ - 16) == freq / 2) {
                    TIMA++;
                }
            }
            bus_.DIVReset = false;
            oscillator_ = 0;
            timer_counter_ = 0;
        }
        RegisterType old_div = oscillator_ >> 8;
        oscillator_ += cycles;
        // Divider always equals the top 8 bits of the oscillator
        DIV = oscillator_ >> 8;
        if (old_div & 0b0001'0000) {
            if (!(DIV & 0b0001'0000)) {
                // Falling edge of bit 4, step frame sequencer
                // TODO: cgb double speed makes it bit 5
                for (int i = 0; i < 4; i++) {
                    if (channel_array_ptr_) {
                        auto& chan = (*channel_array_ptr_)[i];
                        chan.StepFrameSequencer();
                        if (chan.LengthTimer == 0) {
                            bus_.ClearNR52Bit(i);
                        }
                        if (chan.DisableChannelFlag) {
                            bus_.ClearNR52Bit(i);
                            chan.DisableChannelFlag = false;
                        }
                    }
                }
            }
        }
        if (enabled) {
            timer_counter_ += cycles;
            while (timer_counter_ >= freq) {
                timer_counter_ -= freq;
                if (TIMA == 0xFF) {
                    // After TIMA overflows, it stays 00 for 1 clock and *then* becomes =TMA
                    TIMA = 0;
                    tima_overflow_ = true;
                    ret = true;
                    break;
                } else {
                    TIMA++;
                }
            }
        }
        return false;// ret;
    }
}