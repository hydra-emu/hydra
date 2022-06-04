#pragma once
#ifndef TKP_GENERIC_TOOL_MAKE_H
#define TKP_GENERIC_TOOL_MAKE_H
#include "generic_disassembler.h"
#include "emulator_types.hxx"

namespace TKPEmu::Applications::Type {
    class GenericDisassembler;
}

template<class Tool>
std::unique_ptr<TKPEmu::Applications::Drawable> MakeGenericTool(TKPEmu::EmuType emutype, std::shared_ptr<TKPEmu::Emulator> emulator);

template<>
std::unique_ptr<TKPEmu::Applications::Drawable> MakeGenericTool<TKPEmu::Applications::Type::GenericDisassembler>(TKPEmu::EmuType emutype, std::shared_ptr<TKPEmu::Emulator> emulator) {
    using namespace TKPEmu::Applications;
    switch (emutype) {
        case TKPEmu::EmuType::Gameboy: {
            return std::make_unique<Disassembler<GameboyInstruction<>>>(emulator);
        }
        case TKPEmu::EmuType::N64: {
            return std::make_unique<Disassembler<FixedSizeInstruction<uint32_t>>>(emulator);
        }
        case TKPEmu::EmuType::Chip8: {
            return std::make_unique<Disassembler<FixedSizeInstruction<uint8_t>>>(emulator);
        }
        default: {
            return nullptr;
        }
    }
}
#endif