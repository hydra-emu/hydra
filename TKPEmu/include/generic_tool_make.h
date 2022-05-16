#pragma once
#ifndef TKP_GENERIC_TOOL_MAKE_H
#define TKP_GENERIC_TOOL_MAKE_H
#include "generic_disassembler.h"
#include "emulator_types.hxx"

template<class Tool>
TKPEmu::Applications::Drawable* MakeDisassembler(TKPEmu::EmuType emutype, std::shared_ptr<TKPEmu::Emulator> emulator) {
    using namespace TKPEmu::Applications;
    switch (emutype) {
        case TKPEmu::EmuType::Gameboy: {
            return Disassembler<GameboyInstruction>(std::move(emulator));
        }
    }
}
#endif