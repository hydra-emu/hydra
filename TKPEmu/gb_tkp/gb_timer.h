#pragma once
#ifndef TKP_GB_TIMER_H
#define TKP_GB_TIMER_H
#include "gb_bus.h"
#include "gb_addresses.h"
namespace TKPEmu::Gameboy::Devices {
    class Timer {
    public:
        Timer(Bus* bus);
        void Reset();
        void Update(uint8_t cycles, uint8_t old_if);
    private:
        Bus* bus_;
        RegisterType &DIV, &TIMA, &TAC, &TMA, &IF;
        // TODO: Reduce these temporaries to oscillator_ and timer_counter_ only
        int oscillator_, timer_counter_, div_reset_index_, old_tac_;
        bool tima_overflow_;
        const std::array<const unsigned, 4> interr_times_ { 1024, 16, 64, 256 };
    };
}
#endif