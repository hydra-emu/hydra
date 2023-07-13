#include "n64_rsp.hxx"
#include <include/log.hxx>

#define rdreg (gpr_regs_[instruction_.RType.rd])
#define rsreg (gpr_regs_[instruction_.RType.rs])
#define rtreg (gpr_regs_[instruction_.RType.rt])
#define saval (instruction_.RType.sa)
#define immval (instruction_.IType.immediate)
#define seimmval (static_cast<int64_t>(static_cast<int16_t>(instruction_.IType.immediate)))

namespace hydra::N64 {

    void RSP::ERROR() {
        Logger::Warn("Ran ERROR instruction: {:08x}", instruction_.full);
    }

    void RSP::SPECIAL() {
        (special_table_[instruction_.RType.func])(this);
    }
    
    void RSP::REGIMM() {
        (regimm_table_[instruction_.RType.rt])(this);
    }

    void RSP::s_SLL() {
        rdreg.UW = rtreg.UW << saval;
    }
    
    void RSP::s_SLLV() {
        rdreg.UW = rtreg.UW << (rsreg.UW & 0b11111);
    }

    void RSP::s_SRL() {
        rdreg.UW = rtreg.UW >> saval;
    }

    void RSP::s_SRA() {
        rdreg.W = rtreg.W >> saval;
    }

    void RSP::s_SRAV() {
        rdreg.W = rtreg.W >> (rsreg.UW & 0b11111);
    }

    void RSP::s_SRLV() {
        rdreg.UW = rtreg.UW >> (rsreg.UW & 0b11111);
    }

    void RSP::s_JR() {
        auto jump_addr = rsreg.UW;
        branch_to(jump_addr);
    }

    void RSP::s_JALR() {
        auto jump_addr = rsreg.UW;
        link_register(instruction_.RType.rd);
        branch_to(jump_addr);
    }

    void RSP::s_ADDU() {
        rdreg.UW = rtreg.UW + rsreg.UW;
    }

    void RSP::s_SUBU() {
        rdreg.UW = rsreg.UW - rtreg.UW;
    }

    void RSP::s_SLT() {
        rdreg.UW = rsreg.W < rtreg.W;
    }

    void RSP::s_SLTU() {
        rdreg.UW = rsreg.UW < rtreg.UW;
    }

    void RSP::s_BREAK() {
        status_.halt = true;
        status_.broke = true;
        if (status_.intr_break) {
            Logger::Debug("Raising SP interrupt");
            mi_interrupt_->SP = true;
        }
    }

    void RSP::s_AND() {
        rdreg.UW = rtreg.UW & rsreg.UW;
    }

    void RSP::s_OR() {
        rdreg.UW = rtreg.UW | rsreg.UW;
    }

    void RSP::s_XOR() {
        rdreg.UW = rtreg.UW ^ rsreg.UW;
    }

    void RSP::s_NOR() {
        rdreg.UW = ~(rtreg.UW | rsreg.UW);
    }

    void RSP::r_BGEZ() {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch(rsreg.W >= 0, pc_ + seoffset);
    }

    void RSP::r_BLTZ() {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch(rsreg.W < 0, pc_ + seoffset);
    }

    void RSP::r_BGEZAL() {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch(rsreg.W >= 0, pc_ + seoffset);
        link_register(31);
    }

    void RSP::r_BLTZAL() {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch(rsreg.W < 0, pc_ + seoffset);
        link_register(31);
    }

    void RSP::J() {
        auto jump_addr = instruction_.JType.target << 2;
        branch_to(jump_addr);
    }

    void RSP::JAL() {
        link_register(31);
        J();
    }

    void RSP::BEQ() {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch(rsreg.UW == rtreg.UW, pc_ + seoffset);
    }

    void RSP::BNE() {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch(rsreg.UW != rtreg.UW, pc_ + seoffset);
    }

    void RSP::BLEZ() {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch(rsreg.W <= 0, pc_ + seoffset);
    }

    void RSP::BGTZ() {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch(rsreg.W > 0, pc_ + seoffset);
    }

    void RSP::ADDI() {
        int32_t seimm = static_cast<int16_t>(immval);
        int32_t result = 0;
        bool overflow = __builtin_add_overflow(rsreg.W, seimm, &result);
        rtreg.W = result;

        if (overflow) {
            Logger::Fatal("RSP: ADDI overflowed");
        }
    }

    void RSP::ADDIU() {
        int32_t seimm = static_cast<int16_t>(immval);
        int32_t result = 0;
        __builtin_add_overflow(rsreg.W, seimm, &result);
        rtreg.UW = result;
    }

    void RSP::SLTI() {
        rtreg.UW = rsreg.W < seimmval;
    }

    void RSP::SLTIU() {
        rtreg.UW = rsreg.UW < static_cast<int16_t>(immval);
    }

    void RSP::ANDI() {
        rtreg.UW = rsreg.UW & immval;
    }

    void RSP::ORI() {
        rtreg.UW = rsreg.UW | immval;
    }

    void RSP::XORI() {
        rtreg.UW = rsreg.UW ^ immval;
    }

    void RSP::LUI() {
        int32_t imm = immval << 16;
        rtreg.UW = imm;
    }

    void RSP::COP0() {
        switch (instruction_.RType.rs) {
            // MFC0
            case 0: rtreg.UW = read_hwio(static_cast<RSPHWIO>(instruction_.RType.rd)); break;
            // MTC0
            case 4: return write_hwio(static_cast<RSPHWIO>(instruction_.RType.rd), rtreg.UW);
            default: {
                Logger::Fatal("RSP: COP0 unimplemented function: {}", static_cast<uint8_t>(instruction_.RType.rs));
            }
        }
    }

    void RSP::COP1() {
        Logger::Warn("RSP: COP1 not implemented");
    }

    void RSP::COP2() {
        switch (instruction_.WCType.base) {
            case 0: return MFC2();
            case 2: return CFC2();
            case 4: return MTC2();
            case 6: return CTC2();
            default: {
                (vu_instruction_table_[instruction_.FType.func])(this);
                break;
            }
        }
    }

    void RSP::MFC2() {
        gpr_regs_[instruction_.WCType.vt].UW = get_lane(instruction_.WCType.opcode, instruction_.WCType.element);
    }

    void RSP::CFC2() {
        gpr_regs_[instruction_.WCType.vt].UW = get_control(instruction_.WCType.opcode);
    }

    void RSP::MTC2() {
        set_lane(instruction_.WCType.opcode, instruction_.WCType.element, gpr_regs_[instruction_.WCType.vt].UW);
    }

    void RSP::CTC2() {
        set_control(instruction_.WCType.opcode, gpr_regs_[instruction_.WCType.vt].UW);
    }

    void RSP::LB() {
        int16_t offset = immval;
        int32_t seoffset = offset;
        rtreg.W = static_cast<int8_t>(load_byte(seoffset + rsreg.UW));
    }

    void RSP::LH() {
        int16_t offset = immval;
        int32_t seoffset = offset;
        rtreg.W = static_cast<int16_t>(load_halfword(seoffset + rsreg.UW));
    }

    void RSP::LW() {
        int16_t offset = immval;
        int32_t seoffset = offset;
        rtreg.W = load_word(seoffset + rsreg.UW);
    }

    void RSP::LBU() {
        int16_t offset = immval;
        int32_t seoffset = offset;
        rtreg.UW = load_byte(seoffset + rsreg.UW);
    }

    void RSP::LHU() {
        int16_t offset = immval;
        int32_t seoffset = offset;
        rtreg.UW = load_halfword(seoffset + rsreg.UW);
    }

    void RSP::LWU() {
        int16_t offset = immval;
        int32_t seoffset = offset;
        rtreg.UW = load_word(seoffset + rsreg.UW);
    }

    void RSP::SB() {
        int16_t offset = immval;
        int32_t seoffset = offset;
        auto address = seoffset + rsreg.UW;
        store_byte(address, rtreg.UB._0);
    }

    void RSP::SH() {
        int16_t offset = immval;
        int32_t seoffset = offset;
        auto address = seoffset + rsreg.UW;
        store_halfword(address, rtreg.UH._0);
    }

    void RSP::SW() {
        int16_t offset = immval;
        int32_t seoffset = offset;
        auto address = seoffset + rsreg.UW;
        store_word(address, rtreg.UW);
    }

    void RSP::CACHE() {
        Logger::Fatal("RSP: CACHE not implemented");
    }

}

#undef rdreg
#undef rsreg
#undef rtreg
#undef saval
#undef immval
#undef seimmval