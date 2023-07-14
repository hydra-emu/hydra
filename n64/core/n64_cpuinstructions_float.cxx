#include "n64_cpu.hxx"
#include <bit_cast.hxx>
#include <functional>
#include <log.hxx>

#define rdreg (gpr_regs_[instruction_.RType.rd])
#define rsreg (gpr_regs_[instruction_.RType.rs])
#define rtreg (gpr_regs_[instruction_.RType.rt])
#define ftreg (fpr_regs_[instruction_.FType.ft])
#define fsreg (fpr_regs_[instruction_.FType.fs])
#define fdreg (fpr_regs_[instruction_.FType.fd])
#define saval (instruction_.RType.sa)
#define immval (instruction_.IType.immediate)
#define seimmval (static_cast<int64_t>(static_cast<int16_t>(instruction_.IType.immediate)))
#define fmtval (instruction_.FType.fmt)

namespace hydra::N64
{
    struct cast_bitcast
    {
        template <typename T>
        constexpr uint64_t operator()(T x) const
        {
            if constexpr (sizeof(T) == 4)
            {
                return hydra::bit_cast<uint32_t>(x);
            }
            else
            {
                return hydra::bit_cast<uint64_t>(x);
            }
        }
    };

    struct cast_nocast
    {
        template <typename T>
        constexpr uint64_t operator()(T x) const
        {
            return x;
        }
    };

    struct cast_wcast
    {
        template <typename T>
        constexpr uint64_t operator()(T x) const
        {
            if constexpr (sizeof(T) == 4)
            {
                return static_cast<uint32_t>(x);
            }
            else
            {
                return static_cast<uint64_t>(x);
            }
        }
    };

    struct func_sqrt
    {
        template <typename T>
        constexpr T operator()(T x) const
        {
            return std::sqrt(x);
        }
    };

    struct func_abs
    {
        template <typename T>
        constexpr T operator()(T x) const
        {
            return std::abs(x);
        }
    };

    struct func_round
    {
        template <typename T>
        constexpr T operator()(T x) const
        {
            return std::nearbyintl(x);
        }
    };

    struct func_trunc
    {
        template <typename T>
        constexpr T operator()(T x) const
        {
            return std::trunc(x);
        }
    };

    struct func_ceil
    {
        template <typename T>
        constexpr T operator()(T x) const
        {
            return std::ceil(x);
        }
    };

    struct func_floor
    {
        template <typename T>
        constexpr T operator()(T x) const
        {
            return std::floor(x);
        }
    };

    template <>
    double CPU::get_fpu_reg<double>(int regnum)
    {
        return fpr_regs_[regnum].DOUBLE;
    }

    template <>
    float CPU::get_fpu_reg<float>(int regnum)
    {
        return fpr_regs_[regnum].FLOAT._0;
    }

    template <>
    void CPU::set_fpu_reg<double>(int regnum, double data)
    {
        fpr_regs_[regnum].DOUBLE = data;
    }

    template <>
    void CPU::set_fpu_reg<float>(int regnum, float data)
    {
        fpr_regs_[regnum].FLOAT._0 = data;
    }

    template <>
    uint32_t CPU::get_fpr_reg<uint32_t>(int regnum)
    {
        bool _64bit = CP0Status.FR;
        if (_64bit)
        {
            return fpr_regs_[regnum].UW._0;
        }
        else
        {
            if (regnum & 1)
            {
                return fpr_regs_[regnum & ~1].UW._1;
            }
            else
            {
                return fpr_regs_[regnum].UW._0;
            }
        }
    }

    template <>
    uint64_t CPU::get_fpr_reg<uint64_t>(int regnum)
    {
        bool _64bit = CP0Status.FR;
        if (!_64bit)
        {
            regnum &= ~1;
        }
        return fpr_regs_[regnum].UD;
    }

    template <>
    void CPU::set_fpr_reg<uint32_t>(int regnum, uint32_t val)
    {
        bool _64bit = CP0Status.FR;
        if (_64bit)
        {
            fpr_regs_[regnum].UW._0 = val;
        }
        else
        {
            if (regnum & 1)
            {
                fpr_regs_[regnum & ~1].UW._1 = val;
            }
            else
            {
                fpr_regs_[regnum].UW._0 = val;
            }
        }
    }

    template <>
    void CPU::set_fpr_reg<uint64_t>(int regnum, uint64_t val)
    {
        bool _64bit = CP0Status.FR;
        if (!_64bit)
        {
            regnum &= ~1;
        }
        fpr_regs_[regnum].UD = val;
    }

    void CPU::LWC1()
    {
        if (!CP0Status.CP1)
        {
            return throw_exception(prev_pc_, ExceptionType::CoprocessorUnusable, 1);
        }
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint32_t vaddr = seoffset + rsreg.UW._0;
        set_fpr_reg<uint32_t>(instruction_.FType.ft, load_word(vaddr));
    }

    void CPU::LWC2()
    {
        Logger::Warn("LWC2 not implemented");
    }

    void CPU::LLD()
    {
        Logger::Warn("LLD not implemented");
    }

    void CPU::LDC1()
    {
        if (!CP0Status.CP1)
        {
            return throw_exception(prev_pc_, ExceptionType::CoprocessorUnusable, 1);
        }
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint32_t vaddr = seoffset + rsreg.UW._0;
        set_fpr_reg<uint64_t>(instruction_.FType.ft, load_doubleword(vaddr));
    }

    void CPU::LDC2()
    {
        Logger::Warn("LDC2 not implemented");
    }

    void CPU::SWC1()
    {
        if (!CP0Status.CP1)
        {
            return throw_exception(prev_pc_, ExceptionType::CoprocessorUnusable, 1);
        }
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint32_t vaddr = seoffset + rsreg.UW._0;
        store_word(vaddr, get_fpr_reg<uint32_t>(instruction_.FType.ft));
    }

    void CPU::SWC2()
    {
        Logger::Warn("SWC2 not implemented");
    }

    void CPU::SCD()
    {
        Logger::Warn("SCD not implemented");
    }

    void CPU::SDC1()
    {
        if (!CP0Status.CP1)
        {
            return throw_exception(prev_pc_, ExceptionType::CoprocessorUnusable, 1);
        }
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint32_t vaddr = seoffset + rsreg.UW._0;
        store_doubleword(vaddr, get_fpr_reg<uint64_t>(instruction_.FType.ft));
    }

    void CPU::SDC2()
    {
        Logger::Warn("SDC2 not implemented");
    }

    void CPU::f_CFC1()
    {
        int32_t val = 0;
        switch (instruction_.RType.rd)
        {
            case 0:
            {
                // fcr0 seems to only return 0xA00
                val = 0xA00;
                break;
            }
            case 31:
            {
                val = fcr31_.full;
                break;
            }
            default:
                Logger::Warn("CFC1 mega whoopsie");
        }
        rtreg.D = val;
    }

    void CPU::f_MFC1()
    {
        if (!CP0Status.CP1)
        {
            return throw_exception(prev_pc_, ExceptionType::CoprocessorUnusable, 1);
        }
        int32_t reg = get_fpr_reg<uint32_t>(instruction_.FType.fs);
        rtreg.D = reg;
    }

    void CPU::f_DMFC1()
    {
        if (!CP0Status.CP1)
        {
            return throw_exception(prev_pc_, ExceptionType::CoprocessorUnusable, 1);
        }
        rtreg.UD = get_fpr_reg<uint64_t>(instruction_.FType.fs);
    }

    void CPU::f_MTC1()
    {
        if (!CP0Status.CP1)
        {
            return throw_exception(prev_pc_, ExceptionType::CoprocessorUnusable, 1);
        }
        set_fpr_reg<uint32_t>(instruction_.RType.rd, rtreg.W._0);
    }

    void CPU::f_DMTC1()
    {
        if (!CP0Status.CP1)
        {
            return throw_exception(prev_pc_, ExceptionType::CoprocessorUnusable, 1);
        }
        set_fpr_reg(instruction_.RType.rd, rtreg.UD);
    }

    void CPU::f_CTC1()
    {
        switch (instruction_.RType.rd)
        {
            case 0:
            {
                Logger::Warn("CTC1 fcr0 is read only");
                break;
            }
            case 31:
            {
                fcr31_.full = rtreg.UW._0 & 0x183ffff;
                check_fpu_exception();
                break;
            }
            default:
                Logger::Warn("CTC1 mega whoopsie");
        }
    }

    enum { FMT_S = 16, FMT_D = 17, FMT_W = 20, FMT_L = 21 };

    bool CPU::check_fpu_exception()
    {
        bool fire = false;
        if (fcr31_.unimplemented)
        {
            fire = true;
        }
        else if ((fcr31_.cause_inexact && fcr31_.enable_inexact) ||
                 (fcr31_.cause_underflow && fcr31_.enable_underflow) ||
                 (fcr31_.cause_overflow && fcr31_.enable_overflow) ||
                 (fcr31_.cause_divbyzero && fcr31_.enable_divbyzero) ||
                 (fcr31_.cause_invalidop && fcr31_.enable_invalidop))
        {
            fire = true;
        }
        if (fire)
        {
            throw_exception(prev_pc_, ExceptionType::FloatingPoint, 0);
        }
        return fire;
    }

    template <class Type, class OperatorFunction, class CastFunction>
    void CPU::fpu_operate_impl(OperatorFunction op, CastFunction cast)
    {
        bool _64bit = CP0Status.FR;
        Type ft = get_fpu_reg<Type>(instruction_.FType.ft);
        auto fs_index = instruction_.FType.fs;
        if (!_64bit)
        {
            fs_index &= ~1;
        }
        Type fs = get_fpu_reg<Type>(fs_index);
        check_fpu_arg(fs);
        set_fpu_reg<Type>(instruction_.FType.fs, fs);
        if (check_fpu_exception())
        {
            return;
        }
        // if takes 1 parameter
        if constexpr (!std::is_invocable<decltype(op), Type>())
        {
            check_fpu_arg(ft);
            set_fpu_reg<Type>(instruction_.FType.ft, ft);
            if (check_fpu_exception())
            {
                return;
            }
        }
        int round_mode = fegetround();
        switch (fcr31_.rounding_mode)
        {
            case 0:
            {
                fesetround(FE_TONEAREST);
                break;
            }
            case 1:
            {
                fesetround(FE_TOWARDZERO);
                break;
            }
            case 2:
            {
                fesetround(FE_UPWARD);
                break;
            }
            case 3:
            {
                fesetround(FE_DOWNWARD);
                break;
            }
        }
        feclearexcept(FE_ALL_EXCEPT);
        Type result{};
        // if takes 1 parameter
        if constexpr (std::is_invocable<decltype(op), Type>())
        {
            result = std::invoke(op, fs);
        }
        else
        {
            result = std::invoke(op, fs, ft);
        }
        int exception = fetestexcept(FE_ALL_EXCEPT);
        if (exception & FE_UNDERFLOW)
        {
            if (!fcr31_.flush_subnormals || fcr31_.enable_underflow || fcr31_.enable_inexact)
            {
                fcr31_.unimplemented = 1;
                return;
            }
            fcr31_.cause_underflow = 1;
            if (!fcr31_.enable_underflow)
            {
                fcr31_.flag_underflow = 1;
            }
        }
        if (exception & FE_DIVBYZERO)
        {
            fcr31_.cause_divbyzero = 1;
            if (!fcr31_.enable_divbyzero)
            {
                fcr31_.flag_divbyzero = 1;
            }
        }
        if (exception & FE_OVERFLOW)
        {
            fcr31_.cause_overflow = 1;
            if (!fcr31_.enable_overflow)
            {
                fcr31_.flag_overflow = 1;
            }
        }
        if (exception & FE_INEXACT)
        {
            fcr31_.cause_inexact = 1;
            if (!fcr31_.enable_inexact)
            {
                fcr31_.flag_inexact = 1;
            }
        }
        if (exception & FE_INVALID)
        {
            fcr31_.cause_invalidop = 1;
            if (!fcr31_.enable_invalidop)
            {
                fcr31_.flag_invalidop = 1;
            }
        }
        fesetround(round_mode);
        check_fpu_result(result);
        if (check_fpu_exception())
        {
            return;
        }
        fdreg.UD = cast(result);
    }

    template <class OperatorFunction, class CastFunction>
    void CPU::fpu_operate(OperatorFunction op, CastFunction cast)
    {
        fcr31_.cause_inexact = false;
        fcr31_.cause_underflow = false;
        fcr31_.cause_overflow = false;
        fcr31_.cause_divbyzero = false;
        fcr31_.cause_invalidop = false;
        switch (fmtval)
        {
            case FMT_S:
            {
                fpu_operate_impl<float>(op, cast);
                break;
            }
            case FMT_D:
            {
                fpu_operate_impl<double>(op, cast);
                break;
            }
        }
    }

    void CPU::f_ADD()
    {
        fpu_operate(std::plus(), cast_bitcast());
    }

    void CPU::f_SUB()
    {
        fpu_operate(std::minus(), cast_bitcast());
    }

    void CPU::f_MUL()
    {
        fpu_operate(std::multiplies(), cast_bitcast());
    }

    void CPU::f_DIV()
    {
        fpu_operate(std::divides(), cast_bitcast());
    }

    void CPU::f_SQRT()
    {
        fpu_operate(func_sqrt(), cast_bitcast());
    }

    void CPU::f_ABS()
    {
        fpu_operate(func_abs(), cast_bitcast());
    }

    void CPU::f_MOV()
    {
        switch (fmtval)
        {
            case FMT_S:
            {
                int reg = instruction_.FType.fs;
                if (!CP0Status.FR)
                {
                    reg &= ~1;
                }
                uint64_t value = (fpr_regs_[reg]).UD;
                fdreg.UD = value;
                break;
            }
            case FMT_D:
            {
                int reg = instruction_.FType.fs;
                if (!CP0Status.FR)
                {
                    reg &= ~1;
                }
                uint64_t value = (fpr_regs_[reg]).UD;
                fdreg.UD = value;
                break;
            }
            default:
                Logger::Warn("f_MOV: Invalid fmtval");
        }
    }

    void CPU::f_NEG()
    {
        fpu_operate(std::negate(), cast_bitcast());
    }

    void CPU::f_ROUNDL()
    {
        fpu_operate(func_round(), cast_nocast());
    }

    void CPU::f_TRUNCL()
    {
        switch (fmtval)
        {
            case FMT_S:
            {
                uint64_t data = std::trunc(fsreg.FLOAT._0);
                fdreg.UD = hydra::bit_cast<uint64_t>(data);
                break;
            }
            case FMT_D:
            {
                uint64_t data = std::trunc(fsreg.DOUBLE);
                fdreg.UD = hydra::bit_cast<uint64_t>(data);
                break;
            }
        }
    }

    void CPU::f_CEILL()
    {
        fpu_operate(func_ceil(), cast_nocast());
    }

    void CPU::f_FLOORL()
    {
        fpu_operate(func_floor(), cast_nocast());
    }

    void CPU::f_ROUNDW()
    {
        fpu_operate(func_round(), cast_wcast());
    }

    void CPU::f_TRUNCW()
    {
        fpu_operate(func_trunc(), cast_wcast());
    }

    void CPU::f_CEILW()
    {
        fpu_operate(func_ceil(), cast_wcast());
    }

    void CPU::f_FLOORW()
    {
        fpu_operate(func_floor(), cast_wcast());
    }

    void CPU::f_CVTS()
    {
        switch (fmtval)
        {
            case FMT_L:
            {
                int64_t data = fsreg.D;
                fdreg.UD = hydra::bit_cast<uint32_t>(static_cast<float>(data));
                break;
            }
            case FMT_W:
            {
                int32_t data = fsreg.W._0;
                fdreg.UD = hydra::bit_cast<uint32_t>(static_cast<float>(data));
                break;
            }
            case FMT_D:
            {
                double data = fsreg.DOUBLE;
                fdreg.UD = hydra::bit_cast<uint32_t>(static_cast<float>(data));
                break;
            }
            default:
                Logger::Warn("f_CVTS: Invalid fmtval");
        }
    }

    void CPU::f_CVTD()
    {
        switch (fmtval)
        {
            case FMT_L:
            {
                int64_t data = fsreg.D;
                fdreg.UD = hydra::bit_cast<uint64_t>(static_cast<double>(data));
                break;
            }
            case FMT_W:
            {
                int32_t data = fsreg.W._0;
                fdreg.UD = hydra::bit_cast<uint64_t>(static_cast<double>(data));
                break;
            }
            case FMT_S:
            {
                float data = fsreg.FLOAT._0;
                fdreg.UD = hydra::bit_cast<uint64_t>(static_cast<double>(data));
                break;
            }
            default:
                Logger::Warn("f_CVTD: Invalid fmtval");
        }
    }

    void CPU::f_CVTW()
    {
        switch (fmtval)
        {
            case FMT_S:
            {
                float data = fsreg.FLOAT._0;
                fdreg.UD = static_cast<int32_t>(data);
                break;
            }
            case FMT_D:
            {
                double data = fsreg.DOUBLE;
                fdreg.UD = static_cast<int32_t>(data);
                break;
            }
            default:
                Logger::Warn("f_CVTW: Invalid fmtval");
        }
    }

    void CPU::f_CVTL()
    {
        switch (fmtval)
        {
            case FMT_S:
            {
                float data = fsreg.FLOAT._0;
                fdreg.UD = static_cast<int64_t>(data);
                break;
            }
            case FMT_D:
            {
                double data = fsreg.DOUBLE;
                fdreg.UD = static_cast<int64_t>(data);
                break;
            }
            default:
                Logger::Warn("f_CVTL: Invalid fmtval");
        }
    }

    void CPU::f_CF()
    {
        Logger::Warn("f_CF not implemented");
    }

    void CPU::f_CUN()
    {
        switch (fmtval)
        {
            case FMT_S:
            {
                fcr31_.compare = std::isnan(fsreg.FLOAT._0) || std::isnan(ftreg.FLOAT._0);
                break;
            }
            case FMT_D:
            {
                fcr31_.compare = std::isnan(fsreg.DOUBLE) || std::isnan(ftreg.DOUBLE);
                break;
            }
            default:
                Logger::Warn("f_CEQ: Invalid fmtval");
        }
    }

    void CPU::f_CEQ()
    {
        switch (fmtval)
        {
            case FMT_S:
            {
                fcr31_.compare = fsreg.FLOAT._0 == ftreg.FLOAT._0;
                break;
            }
            case FMT_D:
            {
                fcr31_.compare = fsreg.DOUBLE == ftreg.DOUBLE;
                break;
            }
            default:
                Logger::Warn("f_CEQ: Invalid fmtval");
        }
    }

    void CPU::f_CUEQ()
    {
        Logger::Warn("f_CUEQ not implemented");
    }

    void CPU::f_COLT()
    {
        switch (fmtval)
        {
            case FMT_S:
            {
                fcr31_.compare = fsreg.FLOAT._0 < ftreg.FLOAT._0;
                break;
            }
            case FMT_D:
            {
                fcr31_.compare = fsreg.DOUBLE < ftreg.DOUBLE;
                break;
            }
            default:
                Logger::Warn("f_COLT: Invalid fmtval");
        }
    }

    void CPU::f_CULT()
    {
        switch (fmtval)
        {
            case FMT_S:
            {
                fcr31_.compare = fsreg.FLOAT._0 < ftreg.FLOAT._0 ||
                                 (std::isnan(fsreg.FLOAT._0) || std::isnan(ftreg.FLOAT._0));
                break;
            }
            case FMT_D:
            {
                fcr31_.compare = fsreg.DOUBLE < ftreg.DOUBLE ||
                                 (std::isnan(fsreg.DOUBLE) || std::isnan(ftreg.DOUBLE));
                break;
            }
            default:
                Logger::Warn("f_CULT: Invalid fmtval");
        }
    }

    void CPU::f_COLE()
    {
        Logger::Warn("f_COLE not implemented");
    }

    void CPU::f_CULE()
    {
        Logger::Warn("f_CULE not implemented");
    }

    void CPU::f_CSF()
    {
        Logger::Warn("f_CSF not implemented");
    }

    void CPU::f_CNGLE()
    {
        Logger::Warn("f_CNGLE not implemented");
    }

    void CPU::f_CSEQ()
    {
        Logger::Warn("f_CSEQ not implemented");
    }

    void CPU::f_CNGL()
    {
        Logger::Warn("f_CNGL not implemented");
    }

    void CPU::f_CLT()
    {
        switch (fmtval)
        {
            case FMT_S:
            {
                fcr31_.compare = fsreg.FLOAT._0 < ftreg.FLOAT._0;
                break;
            }
            case FMT_D:
            {
                fcr31_.compare = fsreg.DOUBLE < ftreg.DOUBLE;
                break;
            }
            default:
                Logger::Warn("f_CLT: Invalid fmtval");
        }
    }

    void CPU::f_CNGE()
    {
        Logger::Warn("f_CNGE not implemented");
    }

    void CPU::f_CLE()
    {
        switch (fmtval)
        {
            case FMT_S:
            {
                fcr31_.compare = fsreg.FLOAT._0 <= ftreg.FLOAT._0;
                break;
            }
            case FMT_D:
            {
                fcr31_.compare = fsreg.DOUBLE <= ftreg.DOUBLE;
                break;
            }
            default:
                Logger::Warn("f_CLE: Invalid fmtval");
        }
    }

    void CPU::f_CNGT()
    {
        Logger::Warn("f_CNGT not implemented");
    }
} // namespace hydra::N64

#undef rdreg
#undef rsreg
#undef rtreg
#undef saval
#undef immval
#undef seimmval