#pragma once
#ifndef TKP_c8_TKPWRAPPER_H
#define TKP_c8_TKPWRAPPER_H
#include <include/emulator.h>
#include <c8/c8_interpreter.hxx>

namespace hydra::c8 {
    using c8Keys = std::array<uint32_t, 16>;
    class Chip8_TKPWrapper : public Emulator {
        TKP_EMULATOR(Chip8_TKPWrapper);
    private:
        Interpreter inter_;
        c8Keys key_mappings_;
    };
}
#endif