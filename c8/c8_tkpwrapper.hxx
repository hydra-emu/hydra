#pragma once

#include <c8/c8_interpreter.hxx>
#include <emulator.hxx>

namespace hydra::c8
{
    using c8Keys = std::array<uint32_t, 16>;

    class Chip8_TKPWrapper : public Emulator
    {
        TKP_EMULATOR(Chip8_TKPWrapper);

    private:
        Interpreter inter_;
        c8Keys key_mappings_;
    };
} // namespace hydra::c8
