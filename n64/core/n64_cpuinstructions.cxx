#include <log.hxx>
#include <n64/core/n64_cpu.hxx>
#include <random>

#define rdreg (gpr_regs_[instruction_.RType.rd])
#define rsreg (gpr_regs_[instruction_.RType.rs])
#define rtreg (gpr_regs_[instruction_.RType.rt])
#define saval (instruction_.RType.sa)
#define immval (instruction_.IType.immediate)
#define seimmval (static_cast<int64_t>(static_cast<int16_t>(instruction_.IType.immediate)))

namespace hydra::N64
{
    void CPU::ERROR()
    {
        Logger::Fatal("Error instruction at PC: {:08X}, instruction: {}", pc_, instruction_.full);
    }

    void CPU::SPECIAL()
    {
        (special_table_[instruction_.RType.func])(this);
    }

    void CPU::REGIMM()
    {
        (regimm_table_[instruction_.RType.rt])(this);
    }

    void CPU::RDHWR()
    {
        throw_exception(prev_pc_, ExceptionType::ReservedInstruction);
    }

    void CPU::COP0()
    {
        execute_cp0_instruction();
    }

    void CPU::COP1()
    {
        // TODO: another LUT?
        // TODO: preserve cause
        if (!CP0Status.CP1)
        {
            return throw_exception(prev_pc_, ExceptionType::CoprocessorUnusable, 1);
        }
        switch (instruction_.RType.rs)
        {
            case 0b00010:
                f_CFC1();
                break;
            case 0b00000:
                f_MFC1();
                break;
            case 0b00001:
                f_DMFC1();
                break;
            case 0b00100:
                f_MTC1();
                break;
            case 0b00101:
                f_DMTC1();
                break;
            case 0b00110:
                f_CTC1();
                break;
            case 0b01000:
                switch (instruction_.RType.rt)
                {
                    case 0:
                    {
                        int16_t offset = immval << 2;
                        int32_t seoffset = offset;
                        conditional_branch(!fcr31_.compare, pc_ + seoffset);
                        was_branch_ = false;
                        break;
                    }
                    case 1:
                    {
                        int16_t offset = immval << 2;
                        int32_t seoffset = offset;
                        conditional_branch(fcr31_.compare, pc_ + seoffset);
                        was_branch_ = false;
                        break;
                    }
                    case 2:
                    {
                        int16_t offset = immval << 2;
                        int32_t seoffset = offset;
                        conditional_branch_likely(!fcr31_.compare, pc_ + seoffset);
                        was_branch_ = false;
                        break;
                    }
                    case 3:
                    {
                        int16_t offset = immval << 2;
                        int32_t seoffset = offset;
                        conditional_branch_likely(fcr31_.compare, pc_ + seoffset);
                        was_branch_ = false;
                        break;
                    }
                    default:
                    {
                        Logger::Fatal("Unimplemented COP1 BC instruction");
                    }
                }
                break;
            case 0b00011:
            case 0b00111:
            {
                if (!CP0Status.CP1)
                {
                    return throw_exception(prev_pc_, ExceptionType::CoprocessorUnusable, 1);
                }
                fcr31_.cause_divbyzero = 0;
                fcr31_.cause_inexact = 0;
                fcr31_.cause_invalidop = 0;
                fcr31_.cause_overflow = 0;
                fcr31_.cause_underflow = 0;
                fcr31_.unimplemented = 1;
                return throw_exception(prev_pc_, ExceptionType::FloatingPoint, 0);
            }
            case 0x9 ... 0xF:
                return throw_exception(prev_pc_, ExceptionType::ReservedInstruction);
            default:
            {
                (float_table_[instruction_.RType.func])(this);
                break;
            }
        }
    }

    void CPU::MFC2()
    {
        if (!CP0Status.CP2)
        {
            return throw_exception(prev_pc_, ExceptionType::CoprocessorUnusable, 2);
        }
        rtreg.D = static_cast<int32_t>(cp2_weirdness_);
    }

    void CPU::DMFC2()
    {
        if (!CP0Status.CP2)
        {
            return throw_exception(prev_pc_, ExceptionType::CoprocessorUnusable, 2);
        }
        rtreg.D = cp2_weirdness_;
    }

    void CPU::MTC2()
    {
        if (!CP0Status.CP2)
        {
            return throw_exception(prev_pc_, ExceptionType::CoprocessorUnusable, 2);
        }
        cp2_weirdness_ = rtreg.UD;
    }

    void CPU::DMTC2()
    {
        if (!CP0Status.CP2)
        {
            return throw_exception(prev_pc_, ExceptionType::CoprocessorUnusable, 2);
        }
        cp2_weirdness_ = rtreg.UD;
    }

    void CPU::CFC2()
    {
        if (!CP0Status.CP2)
        {
            return throw_exception(prev_pc_, ExceptionType::CoprocessorUnusable, 2);
        }
    }

    void CPU::CTC2()
    {
        if (!CP0Status.CP2)
        {
            return throw_exception(prev_pc_, ExceptionType::CoprocessorUnusable, 2);
        }
    }

    void CPU::COP2()
    {
        switch (instruction_.RType.rs)
        {
            case 0:
                return MFC2();
            case 1:
                return DMFC2();
            case 2:
                return CFC2();
            case 4:
                return MTC2();
            case 5:
                return DMTC2();
            case 6:
                return CTC2();
            default:
            {
                if (!CP0Status.CP2)
                {
                    throw_exception(prev_pc_, ExceptionType::CoprocessorUnusable, 2);
                }
                else
                {
                    throw_exception(prev_pc_, ExceptionType::ReservedInstruction, 2);
                }
            }
        }
    }

    void CPU::COP3()
    {
        throw_exception(prev_pc_, ExceptionType::ReservedInstruction, 0);
    }

    void CPU::CACHE()
    {
        // TODO: TLB stuff
    }

    void CPU::ANDI()
    {
        rtreg.UD = rsreg.UD & immval;
    }

    void CPU::ADDI()
    {
        int32_t seimm = static_cast<int16_t>(immval);
        int32_t result = 0;
        bool overflow = __builtin_add_overflow(rsreg.W._0, seimm, &result);
        if (overflow)
        {
            // An integer overflow exception occurs if carries out of bits 30 and 31 differ (2’s
            // complement overflow). The contents of destination register rt is not modified
            // when an integer overflow exception occurs.
            return throw_exception(prev_pc_, ExceptionType::IntegerOverflow);
        }
        rtreg.D = static_cast<int32_t>(result);
    }

    void CPU::ADDIU()
    {
        rtreg.D = static_cast<int32_t>(rsreg.W._0 + static_cast<int16_t>(immval));
    }

    void CPU::DADDI()
    {
        int64_t seimm = static_cast<int16_t>(immval);
        int64_t result = 0;
        bool overflow = __builtin_add_overflow(rsreg.D, seimm, &result);
        if (overflow)
        {
            // An integer overflow exception occurs if carries out of bits 30 and 31 differ (2’s
            // complement overflow). The contents of destination register rt is not modified
            // when an integer overflow exception occurs.
            return throw_exception(prev_pc_, ExceptionType::IntegerOverflow);
        }
        rtreg.D = result;
    }

    void CPU::DADDIU()
    {
        rtreg.D = rsreg.D + static_cast<int16_t>(immval);
    }

    void CPU::LUI()
    {
        int32_t imm = immval << 16;
        uint64_t seimm = static_cast<int64_t>(imm);
        rtreg.UD = seimm;
    }

    void CPU::ORI()
    {
        rtreg.UD = rsreg.UD | immval;
    }

    void CPU::XORI()
    {
        rtreg.UD = rsreg.UD ^ immval;
    }

    void CPU::SLTI()
    {
        int64_t seimm = static_cast<int16_t>(immval);
        rtreg.UD = rsreg.D < seimm;
    }

    void CPU::SLTIU()
    {
        rtreg.UD = rsreg.UD < static_cast<uint64_t>(seimmval);
    }

    void CPU::MTC0()
    {
        set_cp0_register_32(instruction_.RType.rd, rtreg.UW._0);
    }

    void CPU::DMTC0()
    {
        set_cp0_register_64(instruction_.RType.rd, rtreg.UD);
    }

    void CPU::MFC0()
    {
        int32_t value = get_cp0_register_32(instruction_.RType.rd);
        rtreg.D = value;
    }

    void CPU::DMFC0()
    {
        uint64_t value = get_cp0_register_64(instruction_.RType.rd);
        rtreg.UD = value;
    }

    uint32_t CPU::get_cp0_register_32(uint8_t reg)
    {
        switch (reg)
        {
            case CP0_INDEX:
                return cp0_regs_[reg].UW._0 & 0x8000'003F;
            case CP0_COUNT:
                return cpubus_.time_ >> 1;
            case CP0_CAUSE:
            {
                // TODO: instead update whenever mi_interrupt changes
                CP0CauseType newcause;
                newcause.full = cp0_regs_[reg].UD;
                bool interrupt = cpubus_.mi_interrupt_.full & cpubus_.mi_mask_;
                newcause.IP2 = interrupt;
                return newcause.full;
            }
            case CP0_RANDOM:
            {
                uint8_t wired = cp0_regs_[CP0_WIRED].UB._0;
                int start = 0;
                int end = 64;

                if (wired <= 31)
                {
                    start = wired;
                    end = 32 - wired;
                }

                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(start, start + end - 1);
                auto generated_number = dis(gen);
                return generated_number;
            }
            case 7:
            case 21 ... 25:
            case 31:
                return cp0_weirdness_;
            default:
                return cp0_regs_[reg].UW._0;
        }
    }

    uint64_t CPU::get_cp0_register_64(uint8_t reg)
    {
        switch (reg)
        {
            case CP0_ENTRYLO0:
            case CP0_ENTRYLO1:
            case CP0_CONTEXT:
            case CP0_BADVADDR:
            case CP0_ENTRYHI:
            case CP0_STATUS:
            case CP0_EPC:
            case CP0_PRID:
            case CP0_LLADDR:
            case CP0_ERROREPC:
                return cp0_regs_[reg].UD;
            case CP0_XCONTEXT:
                return cp0_regs_[reg].UD & 0xFFFFFFFFFFFFFFF0;
            default:
                Logger::Fatal("Reading 64 bits from COP0 register {}", reg);
                return 0;
        }
    }

    void CPU::set_cp0_register_32(uint8_t reg, uint32_t val)
    {
        cp0_weirdness_ = val;
        int64_t value = static_cast<int32_t>(val);
        switch (reg)
        {
            case CP0_PAGEMASK:
            {
                union PageMaskWrite
                {
                    uint32_t full;

                    struct
                    {
                        uint32_t      : 13;
                        uint32_t mask : 12;
                        uint32_t      : 7;
                    };
                };

                PageMaskWrite newmask;
                {
                    PageMaskWrite unmaskedmask;
                    unmaskedmask.full = value;
                    newmask.mask = unmaskedmask.mask;
                }
                cp0_regs_[reg].UD = newmask.full;
                break;
            }
            case CP0_ENTRYLO0:
            case CP0_ENTRYLO1:
            {
                cp0_regs_[reg].UD = value & 0x3FFF'FFFF;
                break;
            }
            case CP0_CAUSE:
            {
                CP0CauseType newcause;
                newcause.full = value;
                CP0Cause.IP0 = newcause.IP0;
                CP0Cause.IP1 = newcause.IP1;
                break;
            }
            case CP0_COMPARE:
            {
                CP0Cause.IP7 = false;
                cp0_regs_[reg].UD = value;
                break;
            }
            case CP0_COUNT:
            {
                cpubus_.time_ = value << 1;
                break;
            }
            case CP0_CONFIG:
            {
                cp0_regs_[reg].UD &= ~0x0F00800F;
                cp0_regs_[reg].UD |= value & 0x0F00800F;
                break;
            }
            case CP0_CONTEXT:
            {
                CP0Context.full = (value & 0xFFFFFFFFFF800000) | (CP0Context.full & 0x7FFFFF);
                break;
            }
            case CP0_XCONTEXT:
            {
                CP0XContext.full =
                    (value & 0xFFFFFFFE00000000) | (CP0XContext.full & 0x00000001FFFFFFFF);
                break;
            }
            case CP0_ENTRYHI:
            {
                CP0EntryHi.full = value & 0xC00000FFFFFFE0FF;
                break;
            }
            case CP0_STATUS:
            {
                CP0Status.full &= ~0xFF57FFFF;
                CP0Status.full |= value & 0xFF57FFFF;
                break;
            }
            case CP0_PARITYERROR:
            {
                cp0_regs_[reg].UD = value & 0xFF;
                break;
            }
            case CP0_WIRED:
            {
                cp0_regs_[reg].UD = value & 0x3F;
                break;
            }
            case CP0_PRID:
            case CP0_RANDOM:
            case CP0_CACHEERROR:
            case CP0_BADVADDR:
            {
                break;
            }
            default:
            {
                cp0_regs_[reg].UD = value;
                return;
            }
        }
    }

    void CPU::set_cp0_register_64(uint8_t reg, uint64_t value)
    {
        switch (reg)
        {
            case CP0_ENTRYLO0:
            case CP0_ENTRYLO1:
            {
                cp0_regs_[reg].UD = value & 0x3FFF'FFFF;
                break;
            }
            case CP0_CONTEXT:
            {
                cp0_regs_[reg].UD = (value & 0xFFFFFFFFFF800000) | (cp0_regs_[reg].UD & 0x7FFFFF);
                break;
            }
            case CP0_ENTRYHI:
            {
                cp0_regs_[reg].UD = value & 0xC00000FFFFFFE0FF;
                break;
            }
            case CP0_XCONTEXT:
            {
                cp0_regs_[reg].UD =
                    (value & 0xFFFFFFFE00000000) | (cp0_regs_[reg].UD & 0x00000001FFFFFFFF);
                break;
            }
            case CP0_LLADDR:
                value &= 0xFFFF'FFFF;
                [[fallthrough]];
            case CP0_EPC:
            case CP0_ERROREPC:
            {
                cp0_regs_[reg].UD = value;
                break;
            }
            case CP0_CAUSE:
            {
                CP0CauseType newcause;
                newcause.full = value;
                CP0Cause.IP0 = newcause.IP0;
                CP0Cause.IP1 = newcause.IP1;
                break;
            }

            case CP0_INDEX:
            case CP0_RANDOM:
            case CP0_PAGEMASK:
            case CP0_WIRED:
            case CP0_COUNT:
            case CP0_COMPARE:
            case CP0_PRID:
            case CP0_CONFIG:
            case CP0_WATCHLO:
            case CP0_WATCHHI:
            case CP0_PARITYERROR:
            case CP0_CACHEERROR:
            case CP0_TAGLO:
            case CP0_TAGHI:
            default:
            {
                Logger::Warn("Writing 64 bits to COP0 register {}", reg);
                break;
            }
        }
    }
} // namespace hydra::N64

#undef rdreg
#undef rsreg
#undef rtreg
#undef saval
#undef immval
#undef seimmval