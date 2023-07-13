#pragma once
#ifndef TKP_c8_TKPWRAPPER_H
#define TKP_c8_TKPWRAPPER_H
#include <include/emulator.h>
#include <c8/c8_inter.hxx>

namespace hydra::c8 {
	using c8Keys = std::array<uint32_t, 16>;
    class c8 : public Emulator {
        TKP_EMULATOR(c8);
    private:
        Interpreter inter_;
        c8Keys key_mappings_;
    };
}
#endif