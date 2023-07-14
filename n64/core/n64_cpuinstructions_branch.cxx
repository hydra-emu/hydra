#include "n64_cpu.hxx"
#include <include/log.hxx>

#define rdreg (gpr_regs_[instruction_.RType.rd])
#define rsreg (gpr_regs_[instruction_.RType.rs])
#define rtreg (gpr_regs_[instruction_.RType.rt])
#define saval (instruction_.RType.sa)
#define immval (instruction_.IType.immediate)
#define seimmval (static_cast<int64_t>(static_cast<int16_t>(instruction_.IType.immediate)))

namespace hydra::N64 {

    void CPU::BGTZL() {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch_likely(rsreg.D > 0, pc_ + seoffset);
    }
    
    void CPU::BLEZ() {
        int32_t seoffset = static_cast<int16_t>(immval << 2);
        conditional_branch(rsreg.D <= 0, pc_ + seoffset);
    }

    void CPU::BEQ() {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch(rsreg.UD == rtreg.UD, pc_ + seoffset);
    }

    void CPU::BEQL() {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch_likely(rsreg.UD == rtreg.UD, pc_ + seoffset);
    }

    void CPU::BNE() {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch(rsreg.UD != rtreg.UD, pc_ + seoffset);
    }

    void CPU::BNEL() {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch_likely(rsreg.UD != rtreg.UD, pc_ + seoffset);
    }

    void CPU::BLEZL() {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch_likely(rsreg.D <= 0, pc_ + seoffset);
    }

    void CPU::BGTZ() {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch(rsreg.D > 0, pc_ + seoffset);
    }

    void CPU::JAL() {
        link_register(31);
        J();
    }

    void CPU::J() {
        auto jump_addr = instruction_.JType.target;
        // combine first 3 bits of pc and jump_addr shifted left by 2
        branch_to(((pc_ - 4) & 0xF000'0000) | (jump_addr << 2));
    }

    void CPU::s_JALR() {
        link_register(instruction_.RType.rd);
        // Register numbers rs and rd should not be equal, because such an instruction does
        // not have the same effect when re-executed. If they are equal, the contents of rs
        // are destroyed by storing link address. However, if an attempt is made to execute
        // this instruction, an exception will not occur, and the result of executing such an
        // instruction is undefined.
        if (rdreg.UD != rsreg.UD) {
            // throw_exception(prev_pc_, ExceptionType::AddressErrorLoad);
        }
        s_JR();
    }

    void CPU::s_JR() {
        auto jump_addr = rsreg.UD;
        if ((jump_addr & 0b11) != 0) {
            // Since instructions must be word-aligned, a Jump Register instruction must
            // specify a target register (rs) which contains an address whose low-order two bits
            // are zero. If these low-order two bits are not zero, an address exception will occur
            // when the jump target instruction is fetched.
            set_cp0_regs_exception(jump_addr);
            throw_exception(jump_addr, ExceptionType::AddressErrorLoad);
            return;
        }
        branch_to(jump_addr);
    }

    void CPU::r_BLTZ() {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch(rsreg.D < 0, pc_ + seoffset);
    }
    
    void CPU::r_BGEZ() {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch(rsreg.W._0 >= 0, pc_ + seoffset);
    }
    
    void CPU::r_BLTZL() {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch_likely(rsreg.D < 0, pc_ + seoffset);
    }
    
    void CPU::r_BGEZL() {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch_likely(rsreg.W._0 >= 0, pc_ + seoffset);
    }

    void CPU::r_BLTZAL() {
        Logger::Warn("r_BLTZAL not implemented");
    }
    
    void CPU::r_BGEZAL() {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch(rsreg.D >= 0, pc_ + seoffset);
        link_register(31);
    }
    
    void CPU::r_BLTZALL() {
        Logger::Warn("r_BLTZALL not implemented");
    }
    
    void CPU::r_BGEZALL() {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        link_register(31);
        conditional_branch_likely(rsreg.D >= 0, pc_ + seoffset);
    }

}

#undef rdreg
#undef rsreg
#undef rtreg
#undef saval
#undef immval
#undef seimmval