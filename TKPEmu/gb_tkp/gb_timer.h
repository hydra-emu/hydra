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
        bool Update(uint8_t cycles);
    private:
        Bus* bus_;
        RegisterType &DIV, &TIMA, &TAC, &TMA;
        int oscillator_, timer_counter_;
        const std::array<const unsigned, 4> interr_times_ { 1024, 16, 64, 256 };
    };
}
#endif