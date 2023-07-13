#pragma once
#ifndef TKP_NES_NES_H
#define TKP_NES_NES_H
#include <include/emulator.h>
#include "nes_cpu.hxx"
#include "nes_cpubus.hxx"
#include "nes_ppu.hxx"
#include "nes_apu.hxx"

namespace hydra::NES {
    class NES_TKPWrapper : public Emulator {
        TKP_EMULATOR(NES_TKPWrapper);
    private:
        PPU ppu_ {};
        APU apu_ {};
        CPUBus cpubus_ { ppu_, apu_ };
        CPU cpu_ { cpubus_, Paused };
    };
}
#endif