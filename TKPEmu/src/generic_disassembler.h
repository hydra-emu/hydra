/**
 * This is a generic disassembler class to replace the gb disassembler
 * and hopefully to easily work later for other emulators.
 */
#pragma once
#ifndef TKP_GENERIC_DISASSEMBLER_H
#define TKP_GENERIC_DISASSEMBLER_H
#include <cstdint>

namespace TKPEmu::Applications {
    class Drawable {
    public:
        virtual void Draw() = 0;
    };
    template <class InstructionSize = uint8_t> 
    class GameboyInstruction {
    public:
        using size_type = InstructionSize;
    protected:
        constexpr int get_instr_size(InstructionSize instr) {
            return sizeof(InstructionSize) + GameboyInstruction::instr_size_map[instr];
        }
    private:
        constexpr static int instr_size_map[0x100] = {
            0,  2,  0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  1,  0, 
            0,  2,  0,  0,  0,  0,  1,  0,  1,  0,  0,  0,  0,  0,  1,  0, 
            1,  2,  0,  0,  0,  0,  1,  0,  1,  0,  0,  0,  0,  0,  1,  0, 
            1,  2,  0,  0,  0,  0,  1,  0,  1,  0,  0,  0,  0,  0,  1,  0, 
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
            0,  0,  2,  2,  2,  0,  1,  0,  0,  0,  2,  1 , 2,  2,  2,  0,
            0,  0,  2,  0,  2,  0,  1,  0,  0,  0,  2,  0,  2,  0,  1,  0, 
            0,  0,  0,  0,  0,  0,  1,  0,  1,  0,  0,  0,  0,  0,  1,  0, 
            1,  0,  0,  0,  0,  0,  1 , 0,  1 , 0,  0,  0,  0,  0,  1,  0,
        };
    };
    template <class InstructionSize>
    class FixedSizeInstruction {
    protected:
        constexpr int get_instr_size(InstructionSize instr) {
            return sizeof(InstructionSize);
        }
    public:
        using size_type = InstructionSize;
    };
    template <
        class InstructionPolicy
    >
    class Disassembler : public Drawable, public InstructionPolicy {
    public:
        void Draw() override {
            
        }
    private:
    };
}
#endif