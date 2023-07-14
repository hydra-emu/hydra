#include "n64_cpu.hxx"
#include <include/log.hxx>

#define rdreg (gpr_regs_[instruction_.RType.rd])
#define rsreg (gpr_regs_[instruction_.RType.rs])
#define rtreg (gpr_regs_[instruction_.RType.rt])
#define saval (instruction_.RType.sa)
#define immval (instruction_.IType.immediate)
#define seimmval (static_cast<int64_t>(static_cast<int16_t>(instruction_.IType.immediate)))

constexpr uint64_t LUT[] = {
    0, 0xFF, 0xFFFF, 0, 0xFFFFFFFF, 0, 0, 0, 0xFFFFFFFFFFFFFFFF,
};

namespace hydra::N64 {

    void CPU::LDL() {
        int16_t offset = immval;
        uint64_t address = rsreg.D + offset;
        int shift = 8 * ((address ^ 0) & 7);
        uint64_t mask = (uint64_t)0xFFFFFFFFFFFFFFFF << shift;
        uint64_t data = load_doubleword(address & ~7);
        rtreg.UD = (rtreg.UD & ~mask) | (data << shift);
    }
    
    void CPU::LDR() {
        int16_t offset = immval;
        uint64_t address = rsreg.D + offset;
        int shift = 8 * ((address ^ 7) & 7);
        uint64_t mask = (uint64_t)0xFFFFFFFFFFFFFFFF >> shift;
        uint64_t data = load_doubleword(address & ~7);
        rtreg.UD = (rtreg.UD & ~mask) | (data >> shift);
    }

    void CPU::LWL() {
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint64_t address = seoffset + rsreg.UD;
        uint32_t shift = 8 * ((address ^ 0) & 3);
        uint32_t mask = 0xFFFFFFFF << shift;
        uint32_t data = load_word(address & ~3);
        rtreg.UD = static_cast<int64_t>(static_cast<int32_t>((rtreg.UW._0 & ~mask) | data << shift));
    }
    
    void CPU::LWR() {
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint64_t address = seoffset + rsreg.UD;
        uint32_t shift = 8 * ((address ^ 3) & 3);
        uint32_t mask = 0xFFFFFFFF >> shift;
        uint32_t data = load_word(address & ~3);
        rtreg.UD = static_cast<int64_t>(static_cast<int32_t>((rtreg.UW._0 & ~mask) | data >> shift));
    }
    
    void CPU::SB() {
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint64_t address = seoffset + rsreg.UD;
        store_byte(address, rtreg.UB._0);
    }
    
    void CPU::SWL() {
        constexpr static uint32_t mask[4] = { 0x00000000, 0xFF000000, 0xFFFF0000, 0xFFFFFF00 };
        constexpr static uint32_t shift[4] = { 0, 8, 16, 24 };
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint64_t address = (static_cast<uint32_t>(seoffset) & ~0b11) + rsreg.UD;
        auto addr_off = immval & 0b11;
        uint32_t word = load_word(address);
        word &= mask[addr_off];
        word |= rtreg.UW._0 >> shift[addr_off];
        store_word(address, word);
    }

    void CPU::SWR() {
        constexpr static uint32_t mask[4] = { 0x00FFFFFF, 0x0000FFFF, 0x000000FF, 0x00000000 };
        constexpr static uint32_t shift[4] = { 24, 16, 8, 0 };
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint64_t address = (static_cast<uint32_t>(seoffset) & ~0b11) + rsreg.UD;
        auto addr_off = immval & 0b11;
        uint32_t word = load_word(address);
        word &= mask[addr_off];
        word |= rtreg.UW._0 << shift[addr_off];
        store_word(address, word);
    }
    
    void CPU::SDL() {
        constexpr static uint64_t mask[8] = { 0, 0xFF00000000000000, 0xFFFF000000000000, 0xFFFFFF0000000000, 0xFFFFFFFF00000000, 0xFFFFFFFFFF000000, 0xFFFFFFFFFFFF0000, 0xFFFFFFFFFFFFFF00 };
        constexpr static uint32_t shift[8] = { 0, 8, 16, 24, 32, 40, 48, 56 };
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint64_t address = (static_cast<uint32_t>(seoffset) & ~0b111) + rsreg.UD;
        auto addr_off = immval & 0b111;
        uint64_t doubleword = load_doubleword(address);
        doubleword &= mask[addr_off];
        doubleword |= rtreg.UD >> shift[addr_off];
        store_doubleword(address, doubleword);
    }

    void CPU::SDR() {
        constexpr static uint64_t mask[8] = { 0x00FFFFFFFFFFFFFF, 0x0000FFFFFFFFFFFF, 0x000000FFFFFFFFFF, 0x00000000FFFFFFFF, 0x0000000000FFFFFF, 0x000000000000FFFF, 0x00000000000000FF, 0x0000000000000000 };
        constexpr static uint32_t shift[8] = { 56, 48, 40, 32, 24, 16, 8, 0 };
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint64_t address = (static_cast<uint32_t>(seoffset) & ~0b111) + rsreg.UD;
        auto addr_off = immval & 0b111;
        uint64_t doubleword = load_doubleword(address);
        doubleword &= mask[addr_off];
        doubleword |= rtreg.UD << shift[addr_off];
        store_doubleword(address, doubleword);
    }

    void CPU::SD() {
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint64_t address = seoffset + rsreg.UD;
        if ((address & 0b111) != 0) { 
            // If either of the loworder two bits of the address are not zero, an address error exception occurs.
            set_cp0_regs_exception(address);
            throw_exception(prev_pc_, ExceptionType::AddressErrorStore);
            return;
        }
        if (!mode64_ && opmode_ != OperatingMode::Kernel) {
            // This operation is defined for the VR4300 operating in 64-bit mode and in 32-bit
            // Kernel mode. Execution of this instruction in 32-bit User or Supervisor mode
            // causes a reserved instruction exception.
            throw_exception(prev_pc_, ExceptionType::ReservedInstruction);
        }
        store_doubleword(address, rtreg.UD);
    }

    void CPU::SW() {
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint64_t address = seoffset + rsreg.UD;
        if ((address & 0b11) != 0) {
            // If either of the loworder two bits of the address are not zero, an address error exception occurs.
            set_cp0_regs_exception(address);
            throw_exception(prev_pc_, ExceptionType::AddressErrorStore);
            return;
        }
        if ((address >> 31) && static_cast<int64_t>(address) > 0) {
            // If bit 31 is set and address is positive, that means it's not sign extended, an address error exception occurs.
            set_cp0_regs_exception(address);
            throw_exception(prev_pc_, ExceptionType::AddressErrorStore);
            return;
        }
        store_word(address, rtreg.UW._0);
    }

    void CPU::SH() {
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint64_t address = seoffset + rsreg.UD;
        if ((address & 0b1) != 0) {
            // If either of the loworder two bits of the address are not zero, an address error exception occurs.
            set_cp0_regs_exception(address);
            throw_exception(prev_pc_, ExceptionType::AddressErrorStore);
            return;
        }
        store_halfword(address, rtreg.UH._0);
    }

    void CPU::SC() {
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint64_t address = seoffset + rsreg.UD;

        if ((address & 0b11) != 0) {
            // If either of the loworder two bits of the address are not zero, an address error exception occurs.
            set_cp0_regs_exception(address);
            throw_exception(prev_pc_, ExceptionType::AddressErrorStore);
            return;
        }

        if (llbit_) {
            store_word(address, rtreg.UW._0);
            rtreg.UD = 1;
        } else {
            rtreg.UD = 0;
        }
    }

    void CPU::LBU() {
        int16_t offset = immval;
        int32_t seoffset = offset;
        rtreg.UD = load_byte(seoffset + rsreg.UD);
    }

    void CPU::LB() {
        int16_t offset = immval;
        int32_t seoffset = offset;
        rtreg.D = static_cast<int8_t>(load_byte(seoffset + rsreg.UD));
    }

    void CPU::LHU() {
        int16_t offset = immval;
        int32_t seoffset = offset;
        rtreg.UD = load_halfword(seoffset + rsreg.UD);
    }

    void CPU::LH() {
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint64_t address = seoffset + rsreg.UD;
        if ((address & 0b1) != 0) {
            // If the least-significant bit of the address is not zero, an address error exception occurs.
            set_cp0_regs_exception(address);
            throw_exception(prev_pc_, ExceptionType::AddressErrorLoad);
            return;
        }
        rtreg.D = static_cast<int16_t>(load_halfword(address));
    }

    void CPU::LWU() {
        int16_t offset = immval;
        int32_t seoffset = offset;
        rtreg.UD = load_word(seoffset + rsreg.UD);
    }

    void CPU::LW() {
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint64_t address = seoffset + rsreg.UD;
        if ((address & 0b11) != 0) {
            // If either of the loworder two bits of the address are not zero, an address error exception occurs.
            set_cp0_regs_exception(address);
            throw_exception(prev_pc_, ExceptionType::AddressErrorLoad);
            return;
        }
        if ((address >> 31) && static_cast<int64_t>(address) > 0) {
            // If bit 31 is set and address is positive, that means it's not sign extended, an address error exception occurs.
            set_cp0_regs_exception(address);
            throw_exception(prev_pc_, ExceptionType::AddressErrorLoad);
            return;
        }
        rtreg.D = static_cast<int32_t>(load_word(address));
    }

    void CPU::LD() {
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint64_t address = seoffset + rsreg.UD;
        if ((address & 0b111) != 0) { 
            // If either of the loworder two bits of the address are not zero, an address error exception occurs.
            set_cp0_regs_exception(address);
            throw_exception(prev_pc_, ExceptionType::AddressErrorLoad);
            return;
        }
        if (!mode64_ && opmode_ != OperatingMode::Kernel) {
            // This operation is defined for the VR4300 operating in 64-bit mode and in 32-bit
            // Kernel mode. Execution of this instruction in 32-bit User or Supervisor mode
            // causes a reserved instruction exception.
            throw_exception(prev_pc_, ExceptionType::ReservedInstruction);
            return;
        }
        rtreg.UD = load_doubleword(address);
    }

    void CPU::LL() {
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint64_t address = seoffset + rsreg.UD;
        if ((address & 0b11) != 0) {
            // If either of the loworder two bits of the address are not zero, an address error exception occurs.
            set_cp0_regs_exception(address);
            throw_exception(prev_pc_, ExceptionType::AddressErrorLoad);
            return;
        }
        rtreg.D = static_cast<int32_t>(load_word(address));
        llbit_ = true;
        lladdr_ = translate_vaddr(address).paddr;
    }


}

#undef rdreg
#undef rsreg
#undef rtreg
#undef saval
#undef immval
#undef seimmval