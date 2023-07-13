#include "nes_cpu.hxx"
#include <stdexcept>
#include <fmt/printf.h>
constexpr uint8_t C = 0;
constexpr uint8_t Z = 1;
constexpr uint8_t I = 2;
constexpr uint8_t D = 3;
constexpr uint8_t B = 4;
constexpr uint8_t U = 5;
constexpr uint8_t V = 6;
constexpr uint8_t N = 7;
#define check_nz(x) P.set(Z, x == 0); P.set(N, x & 0x80)
namespace hydra::NES {
    CPU::CPU(CPUBus& bus, std::atomic_bool& paused) : bus_(bus), paused_(paused) {};

    void CPU::SetKeys(std::unordered_map<uint32_t, Button> keys) {
        keys_ = keys;
    }

    void CPU::Implied() {
        // Do nothing
    }

    void CPU::Immediate() {
        data_ = read(PC++);
    }

    void CPU::Indirect() {
        auto b1 = read(PC++);
        uint16_t b2 = read(PC++);
        auto ind = b1 | (b2 << 8);
        auto b1_p = read(ind);
        uint16_t b2_p = read(((ind + 1) & 0xFF) | (ind & 0xFF00));
        addr_ = b1_p | (b2_p << 8);
        data_ = read(addr_);
    }

    void CPU::IndirectX() {
        auto b1 = read(PC++);
        uint16_t b1_p = read((b1 + X) & 0xFF);
        uint16_t b2_p = read((b1 + X + 1) & 0xFF);
        addr_ = b1_p | (b2_p << 8);
        data_ = read_no_d(addr_);
    }

    void CPU::IndirectY() {
        auto b1 = read(PC++);
        uint16_t b1_p = read(b1);
        uint16_t b2_p = read((b1 + 1) & 0xFF);
        addr_ = (b1_p | (b2_p << 8)) + Y;
        data_ = read_no_d(addr_);
    }

    void CPU::ZeroPage() {
        addr_ = read(PC++);
        data_ = read(addr_);
    }

    void CPU::ZeroPageX() {
        addr_ = (read(PC++) + X) & 0xFF;
        data_ = read(addr_);
    }

    void CPU::ZeroPageY() {
        addr_ = (read(PC++) + Y) & 0xFF;
        data_ = read(addr_);
    }

    void CPU::Absolute() {
        auto b1 = read(PC++);
        uint16_t b2 = read(PC++);
        addr_ = b1 | (b2 << 8);
        data_ = read(addr_);
    }

    void CPU::AbsoluteX() {
        auto b1 = read(PC++);
        uint16_t b2 = read(PC++);
        addr_ = (b1 | (b2 << 8)) + X;
        delay((addr_ >> 8) != b2);
        data_ = read(addr_);
    }
    
    void CPU::AbsoluteY() {
        auto b1 = read(PC++);
        uint16_t b2 = read(PC++);
        addr_ = (b1 | (b2 << 8)) + Y;
        delay((addr_ >> 8) != b2);
        data_ = read(addr_);
    }

    void CPU::LDX() {
        X = data_;
        check_nz(X);
    }

    void CPU::LDY() {
        Y = data_;
        check_nz(Y);
    }

    void CPU::LAX() {
        A = data_;
        X = data_;
        check_nz(A);
    }

    void CPU::CPX() {
        check_nz(X - data_);
        P.set(C, X >= data_);
    }

    void CPU::CPY() {
        check_nz(Y - data_);
        P.set(C, Y >= data_);
    }

    void CPU::CMP() {
        check_nz(A - data_);
        P.set(C, A >= data_);
    }

    void CPU::LDA() {
        A = data_;
        check_nz(A);
    }

    void CPU::AND() {
        A &= data_;
        check_nz(A);
    }

    void CPU::ORA() {
        A |= data_;
        check_nz(A);
    }

    void CPU::EOR() {
        A ^= data_;
        check_nz(A);
    }

    void CPU::ADC() {
        auto temp = static_cast<uint16_t>(A) + data_ + P.test(C);
        P.set(C, temp > 0xFF);
        P.set(Z, (temp & 0xFF) == 0);
        P.set(V, (~(static_cast<uint16_t>(A) ^ static_cast<uint16_t>(data_)) & (static_cast<uint16_t>(A) ^ static_cast<uint16_t>(temp))) & 0x0080);
        P.set(N, temp & 0x80);
        A = temp;
    }

    void CPU::SBC() {
        data_ = data_ ^ 0xFF;
        ADC();
    }

    void CPU::JMP() {
        PC = addr_;
    }

    void CPU::BIT() {
        P.set(N, data_ & 0b1000'0000);
        P.set(V, data_ & 0b0100'0000);
        P.set(Z, !(A & data_));
    }

    void CPU::NOP() {
        // Do nothing
    }

    void CPU::STX() {
        write(addr_, X);
    }

    void CPU::STY() {
        write(addr_, Y);
    }

    void CPU::STA() {
        write(addr_, A);
    }

    void CPU::SAX() {
        write(addr_, A & X);
    }

    void CPU::LSR() {
        uint8_t temp = data_ >> 1;
        write(addr_, temp);
        check_nz(temp);
        P.set(C, data_ & 1);
    }

    void CPU::ASL() {
        uint8_t temp = data_ << 1;
        write(addr_, temp);
        check_nz(temp);
        P.set(C, data_ & 0x80);
    }

    void CPU::ROR() {
        uint8_t temp = (data_ >> 1) | (static_cast<uint8_t>(P.test(C)) << 7);
        write(addr_, temp);
        check_nz(temp);
        P.set(C, data_ & 1);
    }

    void CPU::ROL() {
        uint8_t temp = (data_ << 1) | P.test(C);
        write(addr_, temp);
        check_nz(temp);
        P.set(C, data_ & 0x80);
    }

    void CPU::INC() {
        uint8_t temp = data_ + 1;
        write(addr_, temp);
        check_nz(temp);
    }

    void CPU::DEC() {
        uint8_t temp = data_ - 1;
        write(addr_, temp);
        check_nz(temp);
    }

    void CPU::DCP() {
        uint8_t temp = data_ - 1;
        write(addr_, temp);
        check_nz(A - temp);
        P.set(C, A >= temp);
    }

    void CPU::ISC() {
        data_ = data_ + 1;
        write(addr_, data_);
        SBC();
    }

    void CPU::SLO() {
        P.set(C, data_ & 0x80);
        uint8_t temp = data_ << 1;
        write(addr_, temp);
        A = A | temp;
        check_nz(A);
    }

    void CPU::RLA() {
        uint8_t old_c = P.test(C);
        P.set(C, data_ & 0x80);
        uint8_t temp = (data_ << 1) | old_c;
        write(addr_, temp);
        A = A & temp;
        check_nz(A);
    }

    void CPU::RRA() {
        uint8_t old_c = P.test(C);
        P.set(C, data_ & 1);
        data_ = (data_ >> 1) | (old_c << 7);
        write(addr_, data_);
        ADC();
    }

    void CPU::SRE() {
        P.set(C, data_ & 1);
        data_ = data_ >> 1;
        write(addr_, data_);
        A = A ^ data_;
        check_nz(A);
    }

    void CPU::SEC() {
        delay(1);
        P.set(C, true);
    }

    void CPU::SEI() {
        delay(1);
        P.set(I, true);
    }

    void CPU::SED() {
        delay(1);
        P.set(D, true);
    }

    void CPU::CLC() {
        delay(1);
        P.set(C, false);
    }

    void CPU::CLD() {
        delay(1);
        P.set(D, false);
    }

    void CPU::CLV() {
        delay(1);
        P.set(V, false);
    }

    void CPU::CLI() {
        delay(1);
        P.set(I, false);
    }

    void CPU::PHA() {
        delay(1);
        push(A);
    }

    void CPU::PHP() {
        delay(1);
        uint8_t p = P.to_ulong() | 0b0011'0000;
        push(p);
    }

    void CPU::PLA() {
        delay(2);
        A = pull();
        check_nz(A);
    }

    void CPU::PLP() {
        delay(2);
        P &= 0b0011'0000;
        P |= pull() & 0b1100'1111;
    }

    void CPU::JSR() {
        auto b1 = read(PC++);
        delay(1);
        push(PC >> 8);
        push(PC & 0xFF);
        uint16_t b2 = read(PC);
        uint16_t addr = b1 | (b2 << 8);
        PC = addr;
    }

    void CPU::RTS() {
        delay(2);
        auto pc_l = pull();
        auto pc_h = pull();
        PC = (pc_l | (pc_h << 8)) + 1;
        delay(1);
    }

    void CPU::RTI() {
        delay(1);
        P &= 0b0011'0000;
        P |= pull() & 0b1100'1111;
        auto pc_l = pull();
        auto pc_h = pull();
        PC = (pc_l | (pc_h << 8));
        delay(1);
    }

    void CPU::TAY() {
        delay(1);
        Y = A;
        check_nz(Y);
    }

    void CPU::TAX() {
        delay(1);
        X = A;
        check_nz(X);
    }

    void CPU::TSX() {
        delay(1);
        X = SP;
        check_nz(X);
    }

    void CPU::TYA() {
        delay(1);
        A = Y;
        check_nz(A);
    }

    void CPU::TXA() {
        delay(1);
        A = X;
        check_nz(A);
    }

    void CPU::TXS() {
        delay(1);
        SP = X;
    }

    void CPU::BCS() {
        auto b1 = read(PC++);
        auto old_pc = PC;
        if (P.test(C)) {
            PC += int8_t(b1);
            delay(1);
            if ((old_pc & 0xFF00) != (PC & 0xFF00))
                delay(1); // page cross penalty
        }
    }

    void CPU::BCC() {
        auto b1 = read(PC++);
        auto old_pc = PC;
        if (!P.test(C)) {
            PC += int8_t(b1);
            delay(1);
            if ((old_pc & 0xFF00) != (PC & 0xFF00))
                delay(1); // page cross penalty
        }
    }

    void CPU::BEQ() {
        auto b1 = read(PC++);
        auto old_pc = PC;
        if (P.test(Z)) {
            PC += int8_t(b1);
            delay(1);
            if ((old_pc & 0xFF00) != (PC & 0xFF00))
                delay(1); // page cross penalty
        }
    }

    void CPU::BNE() {
        auto b1 = read(PC++);
        auto old_pc = PC;
        if (!P.test(Z)) {
            PC += int8_t(b1);
            delay(1);
            if ((old_pc & 0xFF00) != (PC & 0xFF00))
                delay(1); // page cross penalty
        }
    }

    void CPU::BVS() {
        auto b1 = read(PC++);
        auto old_pc = PC;
        if (P.test(V)) {
            PC += int8_t(b1);
            delay(1);
            if ((old_pc & 0xFF00) != (PC & 0xFF00))
                delay(1); // page cross penalty
        }
    }

    void CPU::BVC() {
        auto b1 = read(PC++);
        auto old_pc = PC;
        if (!P.test(V)) {
            PC += int8_t(b1);
            delay(1);
            if ((old_pc & 0xFF00) != (PC & 0xFF00))
                delay(1); // page cross penalty
        }
    }

    void CPU::BPL() {
        auto b1 = read(PC++);
        auto old_pc = PC;
        if (!P.test(N)) {
            PC += int8_t(b1);
            delay(1);
            if ((old_pc & 0xFF00) != (PC & 0xFF00))
                delay(1); // page cross penalty
        }
    }

    void CPU::BMI() {
        auto b1 = read(PC++);
        auto old_pc = PC;
        if (P.test(N)) {
            PC += int8_t(b1);
            delay(1);
            if ((old_pc & 0xFF00) != (PC & 0xFF00))
                delay(1); // page cross penalty
        }
    }

    void CPU::INX() {
        X++;
        check_nz(X);
    }

    void CPU::INY() {
        Y++;
        check_nz(Y);
    }

    void CPU::DEY() {
        Y--;
        check_nz(Y);
    }

    void CPU::DEX() {
        X--;
        check_nz(X);
    }

    void CPU::NMI_impl() {
        push(PC >> 8);
        push(PC & 0xFF);
        P.set(B, false);
        P.set(U, true);
        P.set(I, true);
        uint8_t p = P.to_ulong() & ~0b0011'0000;
        push(p);
        uint16_t b1 = read(0xFFFA);
        uint16_t b2 = read(0xFFFB);
        uint16_t addr = b1 | (b2 << 8);
        PC = addr;
        nmi_queued_ = false;
        was_prefetched_ = false;
    }
    
    void CPU::NMI() {
        nmi_queued_ = true;
    }

    void CPU::BRK() {
        PC += 1;
        push(PC >> 8);
        push(PC & 0xFF);
        uint8_t p = P.to_ulong() | 0b0011'0000;
        push(p);
        P.set(I, true);
        uint16_t b1 = read_no_d(0xFFFE);
        uint16_t b2 = read_no_d(0xFFFF);
        uint16_t addr = b1 | (b2 << 8);
        PC = addr;
    }

    void CPU::LSR_a() {
        P.set(C, A & 1);
        A = A >> 1;
        check_nz(A);
    }

    void CPU::ASL_a() {
        P.set(C, A & 0x80);
        A = A << 1;
        check_nz(A);
    }

    void CPU::ROL_a() {
        data_ = A;
        A = (A << 1) | P.test(C);
        P.set(C, data_ & 0x80);
        check_nz(A);
    }

    void CPU::ROR_a() {
        data_ = A;
        A = (A >> 1) | (static_cast<uint8_t>(P.test(C)) << 7);
        P.set(C, data_ & 1);
        check_nz(A);
    }

    void CPU::fetch() {
        if (!was_prefetched_) [[unlikely]] {
            prefetch();
        }
        was_prefetched_ = false;
        PC++;
    }

    void CPU::prefetch() {
        fetched_ = read_no_d(PC);
        was_prefetched_ = true;
    }

    void CPU::delay(uint8_t i) {
        cycles_ += i;
        for (int j = 0; j < i; j++) {
            bus_.ppu_.Tick();
            bus_.apu_.Tick();
        }
    }

    uint8_t CPU::read_no_d(uint16_t addr) {
        return bus_.read(addr);
    }

    uint8_t CPU::read(uint16_t addr) {
        auto ret = read_no_d(addr);
        delay(1);
        return ret;
    }
    
    void CPU::write(uint16_t addr, uint8_t data) {
        bus_.write(addr, data);
        delay(1);
    }

    void CPU::push(uint8_t data) {
        write(0x0100 | SP--, data);
    }

    uint8_t CPU::pull() {
        return read(0x0100 | ++SP);
    }

    void CPU::execute() {
        fetch();
        switch (fetched_) {
            case 0x00: Opcode<&CPU::Implied, &CPU::BRK>(this); break;
            case 0x01: Opcode<&CPU::IndirectX, &CPU::ORA>(this); break;
            case 0x03: Opcode<&CPU::IndirectX, &CPU::SLO>(this); break;
            case 0x04: Opcode<&CPU::ZeroPage, &CPU::NOP>(this); break;
            case 0x05: Opcode<&CPU::ZeroPage, &CPU::ORA>(this); break;
            case 0x06: Opcode<&CPU::ZeroPage, &CPU::ASL>(this); break;
            case 0x07: Opcode<&CPU::ZeroPage, &CPU::SLO>(this); break;
            case 0x08: Opcode<&CPU::Implied, &CPU::PHP>(this); break;
            case 0x09: Opcode<&CPU::Immediate, &CPU::ORA>(this); break;
            case 0x0A: ASL_a(); break;
            case 0x0C: Opcode<&CPU::Absolute, &CPU::NOP>(this); break;
            case 0x0D: Opcode<&CPU::Absolute, &CPU::ORA>(this); break;
            case 0x0E: Opcode<&CPU::Absolute, &CPU::ASL>(this); break;
            case 0x0F: Opcode<&CPU::Absolute, &CPU::SLO>(this); break;
            case 0x10: Opcode<&CPU::Implied, &CPU::BPL>(this); break;
            case 0x11: Opcode<&CPU::IndirectY, &CPU::ORA>(this); break;
            case 0x13: Opcode<&CPU::IndirectY, &CPU::SLO>(this); break;
            case 0x14: Opcode<&CPU::ZeroPageX, &CPU::NOP>(this); break;
            case 0x15: Opcode<&CPU::ZeroPageX, &CPU::ORA>(this); break;
            case 0x16: Opcode<&CPU::ZeroPageX, &CPU::ASL>(this); break;
            case 0x17: Opcode<&CPU::ZeroPageX, &CPU::SLO>(this); break;
            case 0x18: Opcode<&CPU::Implied, &CPU::CLC>(this); break;
            case 0x19: Opcode<&CPU::AbsoluteY, &CPU::ORA>(this); break;
            case 0x1A: Opcode<&CPU::Implied, &CPU::NOP>(this); break;
            case 0x1B: Opcode<&CPU::AbsoluteY, &CPU::SLO>(this); break;
            case 0x1C: Opcode<&CPU::AbsoluteX, &CPU::NOP>(this); break;
            case 0x1D: Opcode<&CPU::AbsoluteX, &CPU::ORA>(this); break;
            case 0x1E: Opcode<&CPU::AbsoluteX, &CPU::ASL>(this); break;
            case 0x1F: Opcode<&CPU::AbsoluteX, &CPU::SLO>(this); break;
            case 0x20: Opcode<&CPU::Implied, &CPU::JSR>(this); break;
            case 0x21: Opcode<&CPU::IndirectX, &CPU::AND>(this); break;
            case 0x23: Opcode<&CPU::IndirectX, &CPU::RLA>(this); break;
            case 0x24: Opcode<&CPU::ZeroPage, &CPU::BIT>(this); break;
            case 0x25: Opcode<&CPU::ZeroPage, &CPU::AND>(this); break;
            case 0x26: Opcode<&CPU::ZeroPage, &CPU::ROL>(this); break;
            case 0x27: Opcode<&CPU::ZeroPage, &CPU::RLA>(this); break;
            case 0x28: Opcode<&CPU::Implied, &CPU::PLP>(this); break;
            case 0x29: Opcode<&CPU::Immediate, &CPU::AND>(this); break;
            case 0x2A: ROL_a(); break;
            case 0x2C: Opcode<&CPU::Absolute, &CPU::BIT>(this); break;
            case 0x2D: Opcode<&CPU::Absolute, &CPU::AND>(this); break;
            case 0x2E: Opcode<&CPU::Absolute, &CPU::ROL>(this); break;
            case 0x2F: Opcode<&CPU::Absolute, &CPU::RLA>(this); break;
            case 0x30: Opcode<&CPU::Implied, &CPU::BMI>(this); break;
            case 0x31: Opcode<&CPU::IndirectY, &CPU::AND>(this); break;
            case 0x33: Opcode<&CPU::IndirectY, &CPU::RLA>(this); break;
            case 0x34: Opcode<&CPU::ZeroPageX, &CPU::NOP>(this); break;
            case 0x35: Opcode<&CPU::ZeroPageX, &CPU::AND>(this); break;
            case 0x36: Opcode<&CPU::ZeroPageX, &CPU::ROL>(this); break;
            case 0x37: Opcode<&CPU::ZeroPageX, &CPU::RLA>(this); break;
            case 0x38: Opcode<&CPU::Implied, &CPU::SEC>(this); break;
            case 0x39: Opcode<&CPU::AbsoluteY, &CPU::AND>(this); break;
            case 0x3A: Opcode<&CPU::Implied, &CPU::NOP>(this); break;
            case 0x3B: Opcode<&CPU::AbsoluteY, &CPU::RLA>(this); break;
            case 0x3C: Opcode<&CPU::AbsoluteX, &CPU::NOP>(this); break;
            case 0x3D: Opcode<&CPU::AbsoluteX, &CPU::AND>(this); break;
            case 0x3E: Opcode<&CPU::AbsoluteX, &CPU::ROL>(this); break;
            case 0x3F: Opcode<&CPU::AbsoluteX, &CPU::RLA>(this); break;
            case 0x40: Opcode<&CPU::Implied, &CPU::RTI>(this); break;
            case 0x41: Opcode<&CPU::IndirectX, &CPU::EOR>(this); break;
            case 0x43: Opcode<&CPU::IndirectX, &CPU::SRE>(this); break;
            case 0x44: Opcode<&CPU::ZeroPage, &CPU::NOP>(this); break;
            case 0x45: Opcode<&CPU::ZeroPage, &CPU::EOR>(this); break;
            case 0x46: Opcode<&CPU::ZeroPage, &CPU::LSR>(this); break;
            case 0x47: Opcode<&CPU::ZeroPage, &CPU::SRE>(this); break;
            case 0x48: Opcode<&CPU::Implied, &CPU::PHA>(this); break;
            case 0x49: Opcode<&CPU::Immediate, &CPU::EOR>(this); break;
            case 0x4A: LSR_a(); break;
            case 0x4C: Opcode<&CPU::Absolute, &CPU::JMP>(this); break;
            case 0x4D: Opcode<&CPU::Absolute, &CPU::EOR>(this); break;
            case 0x4E: Opcode<&CPU::Absolute, &CPU::LSR>(this); break;
            case 0x4F: Opcode<&CPU::Absolute, &CPU::SRE>(this); break;
            case 0x50: Opcode<&CPU::Implied, &CPU::BVC>(this); break;
            case 0x51: Opcode<&CPU::IndirectY, &CPU::EOR>(this); break;
            case 0x53: Opcode<&CPU::IndirectY, &CPU::SRE>(this); break;
            case 0x54: Opcode<&CPU::ZeroPageX, &CPU::NOP>(this); break;
            case 0x55: Opcode<&CPU::ZeroPageX, &CPU::EOR>(this); break;
            case 0x56: Opcode<&CPU::ZeroPageX, &CPU::LSR>(this); break;
            case 0x57: Opcode<&CPU::ZeroPageX, &CPU::SRE>(this); break;
            case 0x58: Opcode<&CPU::Implied, &CPU::CLI>(this); break;
            case 0x59: Opcode<&CPU::AbsoluteY, &CPU::EOR>(this); break;
            case 0x5A: Opcode<&CPU::Implied, &CPU::NOP>(this); break;
            case 0x5B: Opcode<&CPU::AbsoluteY, &CPU::SRE>(this); break;
            case 0x5C: Opcode<&CPU::AbsoluteX, &CPU::NOP>(this); break;
            case 0x5D: Opcode<&CPU::AbsoluteX, &CPU::EOR>(this); break;
            case 0x5E: Opcode<&CPU::AbsoluteX, &CPU::LSR>(this); break;
            case 0x5F: Opcode<&CPU::AbsoluteX, &CPU::SRE>(this); break;
            case 0x60: Opcode<&CPU::Implied, &CPU::RTS>(this); break;
            case 0x61: Opcode<&CPU::IndirectX, &CPU::ADC>(this); break;
            case 0x63: Opcode<&CPU::IndirectX, &CPU::RRA>(this); break;
            case 0x64: Opcode<&CPU::ZeroPage, &CPU::NOP>(this); break;
            case 0x65: Opcode<&CPU::ZeroPage, &CPU::ADC>(this); break;
            case 0x66: Opcode<&CPU::ZeroPage, &CPU::ROR>(this); break;
            case 0x67: Opcode<&CPU::ZeroPage, &CPU::RRA>(this); break;
            case 0x68: Opcode<&CPU::Implied, &CPU::PLA>(this); break;
            case 0x69: Opcode<&CPU::Immediate, &CPU::ADC>(this); break;
            case 0x6A: ROR_a(); break;
            case 0x6C: Opcode<&CPU::Indirect, &CPU::JMP>(this); break;
            case 0x6D: Opcode<&CPU::Absolute, &CPU::ADC>(this); break;
            case 0x6E: Opcode<&CPU::Absolute, &CPU::ROR>(this); break;
            case 0x6F: Opcode<&CPU::Absolute, &CPU::RRA>(this); break;
            case 0x70: Opcode<&CPU::Implied, &CPU::BVS>(this); break;
            case 0x71: Opcode<&CPU::IndirectY, &CPU::ADC>(this); break;
            case 0x73: Opcode<&CPU::IndirectY, &CPU::RRA>(this); break;
            case 0x74: Opcode<&CPU::ZeroPageX, &CPU::NOP>(this); break;
            case 0x75: Opcode<&CPU::ZeroPageX, &CPU::ADC>(this); break;
            case 0x76: Opcode<&CPU::ZeroPageX, &CPU::ROR>(this); break;
            case 0x77: Opcode<&CPU::ZeroPageX, &CPU::RRA>(this); break;
            case 0x78: Opcode<&CPU::Implied, &CPU::SEI>(this); break;
            case 0x79: Opcode<&CPU::AbsoluteY, &CPU::ADC>(this); break;
            case 0x7A: Opcode<&CPU::Implied, &CPU::NOP>(this); break;
            case 0x7B: Opcode<&CPU::AbsoluteY, &CPU::RRA>(this); break;
            case 0x7C: Opcode<&CPU::AbsoluteX, &CPU::NOP>(this); break;
            case 0x7D: Opcode<&CPU::AbsoluteX, &CPU::ADC>(this); break;
            case 0x7E: Opcode<&CPU::AbsoluteX, &CPU::ROR>(this); break;
            case 0x7F: Opcode<&CPU::AbsoluteX, &CPU::RRA>(this); break;
            case 0x80: Opcode<&CPU::Immediate, &CPU::NOP>(this); break;
            case 0x81: Opcode<&CPU::IndirectX, &CPU::STA>(this); break;
            case 0x82: Opcode<&CPU::Immediate, &CPU::NOP>(this); break;
            case 0x83: Opcode<&CPU::IndirectX, &CPU::SAX>(this); break;
            case 0x84: Opcode<&CPU::ZeroPage, &CPU::STY>(this); break;
            case 0x85: Opcode<&CPU::ZeroPage, &CPU::STA>(this); break;
            case 0x86: Opcode<&CPU::ZeroPage, &CPU::STX>(this); break;
            case 0x87: Opcode<&CPU::ZeroPage, &CPU::SAX>(this); break;
            case 0x88: Opcode<&CPU::Implied, &CPU::DEY>(this); break;
            case 0x89: Opcode<&CPU::Immediate, &CPU::NOP>(this); break;
            case 0x8A: Opcode<&CPU::Implied, &CPU::TXA>(this); break;
            case 0x8C: Opcode<&CPU::Absolute, &CPU::STY>(this); break;
            case 0x8D: Opcode<&CPU::Absolute, &CPU::STA>(this); break;
            case 0x8E: Opcode<&CPU::Absolute, &CPU::STX>(this); break;
            case 0x8F: Opcode<&CPU::Absolute, &CPU::SAX>(this); break;
            case 0x90: Opcode<&CPU::Implied, &CPU::BCC>(this); break;
            case 0x91: Opcode<&CPU::IndirectY, &CPU::STA>(this); break;
            case 0x94: Opcode<&CPU::ZeroPageX, &CPU::STY>(this); break;
            case 0x95: Opcode<&CPU::ZeroPageX, &CPU::STA>(this); break;
            case 0x96: Opcode<&CPU::ZeroPageY, &CPU::STX>(this); break;
            case 0x97: Opcode<&CPU::ZeroPageY, &CPU::SAX>(this); break;
            case 0x98: Opcode<&CPU::Implied, &CPU::TYA>(this); break;
            case 0x99: Opcode<&CPU::AbsoluteY, &CPU::STA>(this); break;
            case 0x9A: Opcode<&CPU::Implied, &CPU::TXS>(this); break;
            case 0x9D: Opcode<&CPU::AbsoluteX, &CPU::STA>(this); break;
            case 0xA0: Opcode<&CPU::Immediate, &CPU::LDY>(this); break;
            case 0xA1: Opcode<&CPU::IndirectX, &CPU::LDA>(this); break;
            case 0xA2: Opcode<&CPU::Immediate, &CPU::LDX>(this); break;
            case 0xA3: Opcode<&CPU::IndirectX, &CPU::LAX>(this); break;
            case 0xA4: Opcode<&CPU::ZeroPage, &CPU::LDY>(this); break;
            case 0xA5: Opcode<&CPU::ZeroPage, &CPU::LDA>(this); break;
            case 0xA6: Opcode<&CPU::ZeroPage, &CPU::LDX>(this); break;
            case 0xA7: Opcode<&CPU::ZeroPage, &CPU::LAX>(this); break;
            case 0xA8: Opcode<&CPU::Implied, &CPU::TAY>(this); break;
            case 0xA9: Opcode<&CPU::Immediate, &CPU::LDA>(this); break;
            case 0xAA: Opcode<&CPU::Implied, &CPU::TAX>(this); break;
            case 0xAC: Opcode<&CPU::Absolute, &CPU::LDY>(this); break;
            case 0xAD: Opcode<&CPU::Absolute, &CPU::LDA>(this); break;
            case 0xAE: Opcode<&CPU::Absolute, &CPU::LDX>(this); break;
            case 0xAF: Opcode<&CPU::Absolute, &CPU::LAX>(this); break;
            case 0xB0: Opcode<&CPU::Implied, &CPU::BCS>(this); break;
            case 0xB1: Opcode<&CPU::IndirectY, &CPU::LDA>(this); break;
            case 0xB3: Opcode<&CPU::IndirectY, &CPU::LAX>(this); break;
            case 0xB4: Opcode<&CPU::ZeroPageX, &CPU::LDY>(this); break;
            case 0xB5: Opcode<&CPU::ZeroPageX, &CPU::LDA>(this); break;
            case 0xB6: Opcode<&CPU::ZeroPageY, &CPU::LDX>(this); break;
            case 0xB7: Opcode<&CPU::ZeroPageY, &CPU::LAX>(this); break;
            case 0xB8: Opcode<&CPU::Implied, &CPU::CLV>(this); break;
            case 0xB9: Opcode<&CPU::AbsoluteY, &CPU::LDA>(this); break;
            case 0xBA: Opcode<&CPU::Implied, &CPU::TSX>(this); break;
            case 0xBC: Opcode<&CPU::AbsoluteX, &CPU::LDY>(this); break;
            case 0xBD: Opcode<&CPU::AbsoluteX, &CPU::LDA>(this); break;
            case 0xBE: Opcode<&CPU::AbsoluteY, &CPU::LDX>(this); break;
            case 0xBF: Opcode<&CPU::AbsoluteY, &CPU::LAX>(this); break;
            case 0xC0: Opcode<&CPU::Immediate, &CPU::CPY>(this); break;
            case 0xC1: Opcode<&CPU::IndirectX, &CPU::CMP>(this); break;
            case 0xC2: Opcode<&CPU::Immediate, &CPU::NOP>(this); break;
            case 0xC3: Opcode<&CPU::IndirectX, &CPU::DCP>(this); break;
            case 0xC4: Opcode<&CPU::ZeroPage, &CPU::CPY>(this); break;
            case 0xC5: Opcode<&CPU::ZeroPage, &CPU::CMP>(this); break;
            case 0xC6: Opcode<&CPU::ZeroPage, &CPU::DEC>(this); break;
            case 0xC7: Opcode<&CPU::ZeroPage, &CPU::DCP>(this); break;
            case 0xC8: Opcode<&CPU::Implied, &CPU::INY>(this); break;
            case 0xC9: Opcode<&CPU::Immediate, &CPU::CMP>(this); break;
            case 0xCA: Opcode<&CPU::Implied, &CPU::DEX>(this); break;
            case 0xCC: Opcode<&CPU::Absolute, &CPU::CPY>(this); break;
            case 0xCD: Opcode<&CPU::Absolute, &CPU::CMP>(this); break;
            case 0xCE: Opcode<&CPU::Absolute, &CPU::DEC>(this); break;
            case 0xCF: Opcode<&CPU::Absolute, &CPU::DCP>(this); break;
            case 0xD0: Opcode<&CPU::Implied, &CPU::BNE>(this); break;
            case 0xD1: Opcode<&CPU::IndirectY, &CPU::CMP>(this); break;
            case 0xD3: Opcode<&CPU::IndirectY, &CPU::DCP>(this); break;
            case 0xD4: Opcode<&CPU::ZeroPageX, &CPU::NOP>(this); break;
            case 0xD5: Opcode<&CPU::ZeroPageX, &CPU::CMP>(this); break;
            case 0xD6: Opcode<&CPU::ZeroPageX, &CPU::DEC>(this); break;
            case 0xD7: Opcode<&CPU::ZeroPageX, &CPU::DCP>(this); break;
            case 0xD8: Opcode<&CPU::Implied, &CPU::CLD>(this); break;
            case 0xD9: Opcode<&CPU::AbsoluteY, &CPU::CMP>(this); break;
            case 0xDA: Opcode<&CPU::Implied, &CPU::NOP>(this); break;
            case 0xDB: Opcode<&CPU::AbsoluteY, &CPU::DCP>(this); break;
            case 0xDC: Opcode<&CPU::AbsoluteX, &CPU::NOP>(this); break;
            case 0xDD: Opcode<&CPU::AbsoluteX, &CPU::CMP>(this); break;
            case 0xDE: Opcode<&CPU::AbsoluteX, &CPU::DEC>(this); break;
            case 0xDF: Opcode<&CPU::AbsoluteX, &CPU::DCP>(this); break;
            case 0xE0: Opcode<&CPU::Immediate, &CPU::CPX>(this); break;
            case 0xE1: Opcode<&CPU::IndirectX, &CPU::SBC>(this); break;
            case 0xE2: Opcode<&CPU::Immediate, &CPU::NOP>(this); break;
            case 0xE3: Opcode<&CPU::IndirectX, &CPU::ISC>(this); break;
            case 0xE4: Opcode<&CPU::ZeroPage, &CPU::CPX>(this); break;
            case 0xE5: Opcode<&CPU::ZeroPage, &CPU::SBC>(this); break;
            case 0xE6: Opcode<&CPU::ZeroPage, &CPU::INC>(this); break;
            case 0xE7: Opcode<&CPU::ZeroPage, &CPU::ISC>(this); break;
            case 0xE8: Opcode<&CPU::Implied, &CPU::INX>(this); break;
            case 0xE9: Opcode<&CPU::Immediate, &CPU::SBC>(this); break;
            case 0xEA: Opcode<&CPU::Implied, &CPU::NOP>(this); break;
            case 0xEB: Opcode<&CPU::Immediate, &CPU::SBC>(this); break;
            case 0xEC: Opcode<&CPU::Absolute, &CPU::CPX>(this); break;
            case 0xED: Opcode<&CPU::Absolute, &CPU::SBC>(this); break;
            case 0xEE: Opcode<&CPU::Absolute, &CPU::INC>(this); break;
            case 0xEF: Opcode<&CPU::Absolute, &CPU::ISC>(this); break;
            case 0xF0: Opcode<&CPU::Implied, &CPU::BEQ>(this); break;
            case 0xF1: Opcode<&CPU::IndirectY, &CPU::SBC>(this); break;
            case 0xF3: Opcode<&CPU::IndirectY, &CPU::ISC>(this); break;
            case 0xF4: Opcode<&CPU::ZeroPageX, &CPU::NOP>(this); break;
            case 0xF5: Opcode<&CPU::ZeroPageX, &CPU::SBC>(this); break;
            case 0xF6: Opcode<&CPU::ZeroPageX, &CPU::INC>(this); break;
            case 0xF7: Opcode<&CPU::ZeroPageX, &CPU::ISC>(this); break;
            case 0xF8: Opcode<&CPU::Implied, &CPU::SED>(this); break;
            case 0xF9: Opcode<&CPU::AbsoluteY, &CPU::SBC>(this); break;
            case 0xFA: Opcode<&CPU::Implied, &CPU::NOP>(this); break;
            case 0xFB: Opcode<&CPU::AbsoluteY, &CPU::ISC>(this); break;
            case 0xFC: Opcode<&CPU::AbsoluteX, &CPU::NOP>(this); break;
            case 0xFD: Opcode<&CPU::AbsoluteX, &CPU::SBC>(this); break;
            case 0xFE: Opcode<&CPU::AbsoluteX, &CPU::INC>(this); break;
            case 0xFF: Opcode<&CPU::AbsoluteX, &CPU::ISC>(this); break;
            default: throw std::runtime_error(fmt::format("Unknown opcode: {:02X}", fetched_));
        }
        prefetch();
    }

    void CPU::Tick() {
        execute();
        if (nmi_queued_)
            NMI_impl();
    }
    
    void CPU::Reset() {
        PC = (read_no_d(0xFFFD) << 8) | read_no_d(0xFFFC);
        SP = 0xFD;
        A = 0;
        X = 0;
        Y = 0;
        P = 0x24;
        cycles_ = 7;
        was_prefetched_ = false;
        bus_.Reset();
    }

    void CPU::SoftReset() {
        PC = (read_no_d(0xFFFD) << 8) | read_no_d(0xFFFC);
        SP -= 3;
        P.set(I, 1);
        was_prefetched_ = false;
        bus_.Reset();
    }

    void CPU::HandleKeyDown(uint32_t key) {
        bus_.joypad1_.Press(keys_[key]);
    }
    
    void CPU::HandleKeyUp(uint32_t key) {
        bus_.joypad1_.Release(keys_[key]);
    }
}