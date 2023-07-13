#include "n64_cpu.hxx"
#include <include/log.hxx>

#define rdreg (gpr_regs_[instruction_.RType.rd])
#define rsreg (gpr_regs_[instruction_.RType.rs])
#define rtreg (gpr_regs_[instruction_.RType.rt])
#define saval (instruction_.RType.sa)
#define immval (instruction_.IType.immediate)
#define seimmval (static_cast<int64_t>(static_cast<int16_t>(instruction_.IType.immediate)))

namespace hydra::N64 {

    void CPU::s_SLL() {
        rdreg.D = static_cast<int32_t>(rtreg.UW._0 << saval);
    }

    void CPU::s_SRL() {
        rdreg.D = static_cast<int32_t>(rtreg.UW._0 >> saval);
    }

    void CPU::s_SRA() {
        rdreg.D = static_cast<int32_t>(rtreg.D >> saval);
	}

    void CPU::s_SLLV() {
        rdreg.D = static_cast<int32_t>(rtreg.UW._0 << (rsreg.UD & 0b111111));
	}

    void CPU::s_SRLV() {
        rdreg.D = static_cast<int32_t>(rtreg.UW._0 >> (rsreg.UD & 0b111111));
	}
    
    void CPU::s_SRAV() {
        rdreg.D = static_cast<int32_t>(rtreg.D >> (rsreg.UD & 0b11111));
	}

    void CPU::s_DSRL() {
		rdreg.UD = rtreg.UD >> saval;
	}

    void CPU::s_DSRLV() {
		rdreg.UD = rtreg.UD >> (rsreg.UD & 0b111111);
	}

    void CPU::s_DSLL() {
        rdreg.UD = rtreg.UD << saval;
	}

    void CPU::s_DSLLV() {
        rdreg.UD = rtreg.UD << (rsreg.UD & 0b111111);
    }

    void CPU::s_DSLL32() {
        rdreg.UD = rtreg.UD << (saval + 32);
    }

    void CPU::s_DSRA() {
        rdreg.D = rtreg.D >> saval;
	}

    void CPU::s_DSRA32() {
        rdreg.D = rtreg.D >> (saval + 32);
    }

    void CPU::s_DSRAV() {
		rdreg.D = rtreg.D >> (rsreg.UD & 0b111111);
	}
    
    void CPU::s_DSRL32() {
		rdreg.UD = rtreg.UD >> (saval + 32);
	}

    void CPU::s_ADD() {
        int32_t result = 0;
        bool overflow = __builtin_add_overflow(rtreg.W._0, rsreg.W._0, &result);
        if (overflow) {
            return throw_exception(prev_pc_, ExceptionType::IntegerOverflow);
        }
        rdreg.UD = static_cast<int64_t>(result);
    }

    void CPU::s_ADDU() {
        rdreg.D = static_cast<int32_t>(rsreg.UW._0 + rtreg.UW._0);
    }

    void CPU::s_DADD() {
        int64_t result = 0;
        bool overflow = __builtin_add_overflow(rtreg.D, rsreg.D, &result);
        if (overflow) {
            return throw_exception(prev_pc_, ExceptionType::IntegerOverflow);
        }
        rdreg.D = result;
	}
    
    void CPU::s_DADDU() {
		rdreg.UD = rsreg.UD + rtreg.UD;
	}

    void CPU::s_SUB() {
		int32_t result = 0;
		bool overflow = __builtin_sub_overflow(rsreg.W._0, rtreg.W._0, &result);
        if (overflow) {
            return throw_exception(prev_pc_, ExceptionType::IntegerOverflow);
        }
		rdreg.D = result;
	}
    
    void CPU::s_SUBU() {
		rdreg.D = static_cast<int32_t>(rsreg.UW._0 - rtreg.UW._0);
	}

    void CPU::s_DSUB() {
		int64_t result = 0;
		bool overflow = __builtin_sub_overflow(rsreg.D, rtreg.D, &result);
        if (overflow) {
            return throw_exception(prev_pc_, ExceptionType::IntegerOverflow);
        }
		rdreg.D = result;
	}
    
    void CPU::s_DSUBU() {
		rdreg.UD = rsreg.UD - rtreg.UD;
	}
    
    void CPU::s_MULT() {
		uint64_t res = static_cast<int64_t>(rsreg.W._0) * rtreg.W._0;
        lo_ = static_cast<int64_t>(static_cast<int32_t>(res & 0xFFFF'FFFF));
        hi_ = static_cast<int64_t>(static_cast<int32_t>(res >> 32));
	}
    
    void CPU::s_MULTU() {
		uint64_t res = static_cast<uint64_t>(rsreg.UW._0) * rtreg.UW._0;
        lo_ = static_cast<int64_t>(static_cast<int32_t>(res & 0xFFFF'FFFF));
        hi_ = static_cast<int64_t>(static_cast<int32_t>(res >> 32));
	}

    void CPU::s_DMULT() {
		__uint128_t res = static_cast<__uint128_t>(rsreg.D) * rtreg.D;
        lo_ = static_cast<uint64_t>(res);
        hi_ = res >> 64;
	}
    
    void CPU::s_DMULTU() {
		__uint128_t res = static_cast<__uint128_t>(rsreg.UD) * rtreg.UD;
        lo_ = static_cast<uint64_t>(res);
        hi_ = res >> 64;
	}
    
    void CPU::s_DIV() {
		if (rtreg.W._0 == 0) [[unlikely]]
        {
            lo_ = rsreg.W._0 < 0 ? 1 : -1;
            hi_ = static_cast<int64_t>(rsreg.W._0);
            return;
        }
        // TODO: replace with uint64_t division
        if (rsreg.W._0 == std::numeric_limits<decltype(rsreg.W._0)>::min() && rtreg.W._0 == -1) [[unlikely]] {
            lo_ = static_cast<int64_t>(rsreg.W._0);
            hi_ = 0;
            return;
        }
        lo_ = rsreg.W._0 / rtreg.W._0;
        hi_ = rsreg.W._0 % rtreg.W._0;
	}
    
    void CPU::s_DIVU() {
		if (rtreg.UW._0 == 0) [[unlikely]]
        {
            lo_ = -1;
            hi_ = rsreg.UD;
            return;
        }
        lo_ = rsreg.UW._0 / rtreg.UW._0;
        hi_ = rsreg.UW._0 % rtreg.UW._0;
	}
    
    void CPU::s_DDIV() {
		if (rtreg.D == 0) [[unlikely]]
        {
            bool sign = rsreg.UD >> 63;
            lo_ = sign ? 1 : -1;
            hi_ = rsreg.UD;
            return;
        }
        if (rsreg.D == std::numeric_limits<int64_t>::min() && rtreg.D == -1) {
            lo_ = rsreg.D;
            hi_ = 0;
            return;
        }
        lo_ = rsreg.D / rtreg.D;
        hi_ = rsreg.D % rtreg.D;
	}
    
    void CPU::s_DDIVU() {
		if (rtreg.UD == 0) [[unlikely]]
        {
            lo_ = -1;
            hi_ = rsreg.UD;
            return;
        }
        lo_ = static_cast<int64_t>(rsreg.UD / rtreg.UD);
        hi_ = static_cast<int64_t>(rsreg.UD % rtreg.UD);
	}

    void CPU::s_AND() {
        rdreg.UD = rsreg.UD & rtreg.UD;
    }

    void CPU::s_OR() {
        rdreg.UD = rsreg.UD | rtreg.UD;
	}
    
    void CPU::s_XOR() {
		rdreg.UD = rsreg.UD ^ rtreg.UD;
	}
    
    void CPU::s_NOR() {
        rdreg.UD = ~(rsreg.UD | rtreg.UD);
    }
    
    void CPU::s_TGE() {
        if (rsreg.D >= rtreg.D) {
            throw_exception(prev_pc_, ExceptionType::Trap);
        }
    }

    void CPU::s_TGEU() {
		if (rsreg.UD >= rtreg.UD) {
            throw_exception(prev_pc_, ExceptionType::Trap);
        }
	}
    
    void CPU::s_TLT() {
		if (rsreg.D < rtreg.D) {
            throw_exception(prev_pc_, ExceptionType::Trap);
        }
	}
    
    void CPU::s_TLTU() {
		if (rsreg.UD < rtreg.UD) {
            throw_exception(prev_pc_, ExceptionType::Trap);
        }
	}
    
    void CPU::s_TEQ() {
		if (rsreg.UD == rtreg.UD) {
            throw_exception(prev_pc_, ExceptionType::Trap);
        }
	}
    
    void CPU::s_TNE() {
		if (rsreg.UD != rtreg.UD) {
            throw_exception(prev_pc_, ExceptionType::Trap);
        }
	}

    void CPU::r_TGEI() {
        if (rsreg.D >= seimmval) {
            throw_exception(prev_pc_, ExceptionType::Trap);
        }
    }
    
    void CPU::r_TGEIU() {
        if (rsreg.UD >= seimmval) {
            throw_exception(prev_pc_, ExceptionType::Trap);
        }
    }
    
    void CPU::r_TLTI() {
        if (rsreg.D < seimmval) {
            throw_exception(prev_pc_, ExceptionType::Trap);
        }
    }
    
    void CPU::r_TLTIU() {
        if (rsreg.UD < seimmval) {
            throw_exception(prev_pc_, ExceptionType::Trap);
        }
    }
    
    void CPU::r_TEQI() {
        if (rsreg.UD == seimmval) {
            throw_exception(prev_pc_, ExceptionType::Trap);
        }
    }
    
    void CPU::r_TNEI() {
        if (rsreg.UD != seimmval) {
            throw_exception(prev_pc_, ExceptionType::Trap);
        }
    }

    void CPU::s_SLT() {
        rdreg.UD = rsreg.D < rtreg.D;
    }

    void CPU::s_SLTU() {
        rdreg.UD = rsreg.UD < rtreg.UD;
    }

    void CPU::s_SYSCALL() {
		throw_exception(prev_pc_, ExceptionType::Syscall);
	}
    
    void CPU::s_BREAK() {
		throw_exception(prev_pc_, ExceptionType::Breakpoint);
	}

    void CPU::s_SYNC() {}
    
    void CPU::s_MFHI() {
        rdreg.UD = hi_;
	}
    
    void CPU::s_MTHI() {
        hi_ = rsreg.UD;
	}

    void CPU::s_MFLO() {
		rdreg.UD = lo_;
	}

    void CPU::s_MTLO() {
		lo_ = rsreg.UD;
	}

}

#undef rdreg
#undef rsreg
#undef rtreg
#undef saval
#undef immval
#undef seimmval