#pragma once

#include "nes_cpubus.hxx"
#include "nes_joypad.hxx"
#include <atomic>
#include <bitset>
#include <cstdint>
#include <iostream>
#include <unordered_map>

namespace hydra::NES
{
    class NES_TKPWrapper;
}

namespace hydra::NES
{
    class CPU
    {
    public:
        CPU(CPUBus& bus, std::atomic_bool& paused);
        void Tick();
        void Reset();
        void SoftReset();
        void NMI();
        void HandleKeyDown(uint32_t key);
        void HandleKeyUp(uint32_t key);
        void SetKeys(std::unordered_map<uint32_t, Button> keys);

    private:
        CPUBus& bus_;
        inline void delay(uint8_t i);
        uint8_t A = 0, X = 0, Y = 0, SP = 0;
        std::bitset<8> P = 0;
        uint16_t PC = 0;
        uint64_t cycles_ = 0;
        uint8_t fetched_ = 0;
        bool was_prefetched_ = false;
        bool nmi_queued_ = false;
        std::atomic_bool& paused_;

        uint8_t read_no_d(uint16_t addr);
        uint8_t read(uint16_t addr);
        void push(uint8_t data);
        uint8_t pull();
        void write(uint16_t addr, uint8_t data);
        void NMI_impl();

        inline void fetch();
        inline void prefetch();
        inline void execute();
        friend class hydra::NES::NES_TKPWrapper;

    private:
        // clang-format off
        inline void TAY(), TAX(), TYA(), TXA(), JSR(), SEC(), 
            BCC(), CLC(), BEQ(), BNE(), BVS(), BVC(), BMI(), BPL(),
            RTS(), SEI(), SED(), CLD(), PHP(), PLA(), PLP(), PHA(),
            CLV(), INY(), INX(), DEY(), DEX(), BCS(), TSX(), TXS(),
            RTI(), BRK(), CLI(),
            JMP(), LDX(), STX(), LDA(), STA(), BIT(), CMP(), AND(),
            ORA(), EOR(), ADC(), LDY(), CPY(), CPX(), SBC(), LSR(),
            ASL(), ROR(), ROL(), STY(), INC(), DEC(), NOP(), LAX(),
            SAX(), DCP(), ISC(), SLO(), RLA(), RRA(), SRE(),
            LSR_a(), ASL_a(), ROR_a(), ROL_a();

        inline void Immediate(), Absolute(), AbsoluteX(), AbsoluteY(),
            ZeroPage(), ZeroPageX(), ZeroPageY(), Indirect(), IndirectX(),
            IndirectY(), Implied();

        // clang-format on

        template <auto AddressingMode, auto Function>
        static void Opcode(CPU* cpu)
        {
            (cpu->*AddressingMode)();
            (cpu->*Function)();
        }

        uint8_t data_;
        uint16_t addr_;
        std::unordered_map<uint32_t, Button> keys_;
    };
} // namespace hydra::NES
