#pragma once

#include <array>
#include <cstdint>
#include <limits>
#include <string>
#ifndef __cpp_lib_endian
static_assert(false && "std::endian not found");
#endif

namespace hydra::N64
{
    constexpr uint32_t EMPTY_INSTRUCTION = 0xFFFFFFFF;

    // EntryLo0/1 are the same in 32 and 64 bit mode
    union EntryLo
    {
        uint64_t full;

        struct
        {
            uint64_t G   : 1;
            uint64_t V   : 1;
            uint64_t D   : 1;
            uint64_t C   : 3;
            uint64_t PFN : 20;
            uint64_t     : 38;
        };
    };

    union EntryHi
    {
        uint64_t full;

        struct
        {
            uint64_t ASID : 8;
            uint64_t      : 5;
            uint64_t VPN2 : 27;
            uint64_t      : 22;
            uint64_t R    : 2;
        };
    };

    struct TLBEntry
    {
        uint16_t mask;
        bool G;
        EntryHi entry_hi;
        EntryLo entry_even, entry_odd;
        bool initialized;
    };

    union InstructionBase
    {
        struct
        {
            uint32_t immediate : 16; // immediate value
            uint32_t rt        : 5;  // target (source/destination) register number
            uint32_t rs        : 5;  // source register number (r0-r31)
            uint32_t op        : 6;  // operation code
        } IType;                     // Immediate

        struct
        {
            uint32_t target : 26; // unconditional branch target address
            uint32_t op     : 6;  // operation code
        } JType;                  // Jump

        struct
        {
            uint32_t func : 6; // function field
            uint32_t sa   : 5; // shift amount
            uint32_t rd   : 5; // destination register number
            uint32_t rt   : 5; // target register number
            uint32_t rs   : 5; // source register number
            uint32_t op   : 6; // operation code
        } RType;               // Register

        struct
        {
            uint32_t func : 6; // function field
            uint32_t fd   : 5; // destination register number
            uint32_t fs   : 5; // source register number
            uint32_t ft   : 5; // target register number
            uint32_t fmt  : 5; // floating point format
            uint32_t op   : 6; // operation code
        } FType;               // Floating instruction

        struct
        {
            unsigned offset  : 7;
            unsigned element : 4;
            unsigned opcode  : 5;
            unsigned vt      : 5;
            unsigned base    : 5;
            unsigned op      : 6;
        } WCType; // RSP LWC/SWC

        uint32_t full;
    };

    union MIInterrupt
    {
        uint32_t full = 0;

        struct
        {
            uint32_t SP : 1;
            uint32_t SI : 1;
            uint32_t AI : 1;
            uint32_t VI : 1;
            uint32_t PI : 1;
            uint32_t DP : 1;
            uint32_t    : 26;
        };
    };

    static_assert(sizeof(MIInterrupt) == sizeof(uint32_t));

    union MemDataUnionDW
    {
        int64_t D;
        uint64_t UD;

        struct
        {
            int32_t _0;
            int32_t _1;
        } W;

        struct
        {
            uint32_t _0;
            uint32_t _1;
        } UW;

        struct
        {
            int16_t _0;
            int16_t _1;
            int16_t _2;
            int16_t _3;
        } H;

        struct
        {
            uint16_t _0;
            uint16_t _1;
            uint16_t _2;
            uint16_t _3;
        } UH;

        struct
        {
            int8_t _0;
            int8_t _1;
            int8_t _2;
            int8_t _3;
            int8_t _4;
            int8_t _5;
            int8_t _6;
            int8_t _7;
        } B;

        struct
        {
            uint8_t _0;
            uint8_t _1;
            uint8_t _2;
            uint8_t _3;
            uint8_t _4;
            uint8_t _5;
            uint8_t _6;
            uint8_t _7;
        } UB;

        double DOUBLE;

        struct
        {
            float _0;
            float _1;
        } FLOAT;
    };

    union MemDataUnionW
    {
        int32_t W;
        uint32_t UW;

        struct
        {
            int16_t _0;
            int16_t _1;
        } H;

        struct
        {
            uint16_t _0;
            uint16_t _1;
        } UH;

        struct
        {
            int8_t _0;
            int8_t _1;
            int8_t _2;
            int8_t _3;
        } B;

        struct
        {
            uint8_t _0;
            uint8_t _1;
            uint8_t _2;
            uint8_t _3;
        } UB;
    };

    union FCR31
    {
        uint32_t full;

        struct
        {
            uint32_t rounding_mode    : 2;
            uint32_t flag_inexact     : 1;
            uint32_t flag_underflow   : 1;
            uint32_t flag_overflow    : 1;
            uint32_t flag_divbyzero   : 1;
            uint32_t flag_invalidop   : 1;
            uint32_t enable_inexact   : 1;
            uint32_t enable_underflow : 1;
            uint32_t enable_overflow  : 1;
            uint32_t enable_divbyzero : 1;
            uint32_t enable_invalidop : 1;
            uint32_t cause_inexact    : 1;
            uint32_t cause_underflow  : 1;
            uint32_t cause_overflow   : 1;
            uint32_t cause_divbyzero  : 1;
            uint32_t cause_invalidop  : 1;
            uint32_t unimplemented    : 1;
            uint32_t                  : 5;
            uint32_t compare          : 1;
            uint32_t flush_subnormals : 1;
            uint32_t                  : 7;
        };
    };

    static_assert(sizeof(FCR31) == 4, "FCR31 is not 4 bytes");

    union VUInstruction
    {
        uint32_t full;

        struct
        {
            uint32_t op           : 6;
            uint32_t vd           : 5;
            uint32_t vs           : 5;
            uint32_t vt           : 5;
            uint32_t element      : 4;
            uint32_t _must_be_one : 1;
            uint32_t cop2         : 6;
        };
    };

    static_assert(sizeof(VUInstruction) == 4, "VUInstruction is not 4 bytes");
    const static std::array<std::string, 64> OperationCodes = {
        "nop",   "regimm", "j",     "jal",   "beq",   "bne",    "blez", "bgtz",  "addi",  "addiu",
        "slti",  "stliu",  "andi",  "ori",   "xori",  "lui",    "cop0", "cop1",  "cop2",  "23err",
        "beql",  "bnel",   "blezl", "bgtzl", "daddi", "daddiu", "ldl",  "ldr",   "34err", "35err",
        "36err", "37err",  "lb",    "lh",    "lwl",   "lw",     "lbu",  "lhu",   "lwr",   "lwu",
        "sb",    "sh",     "swl",   "sw",    "sdl",   "sdr",    "swr",  "cache", "ll",    "lwc1",
        "lwc2",  "63err",  "lld",   "ldc1",  "ldc2",  "ld",     "sc",   "swc1",  "swc2",  "73err",
        "scd",   "sdc1",   "sdc2",  "sd",
    };
    const static std::array<std::string, 64> SpecialCodes = {
        "sll",    "s2err",  "srl",    "sra",    "sllv",    "s5err",  "srlv",   "srav",
        "jr",     "jalr",   "s12err", "s13err", "syscall", "break",  "s16err", "sync",
        "mfhi",   "mthi",   "mflo",   "mtlo",   "dsllv",   "s25err", "dsrlv",  "dsrav",
        "mult",   "multu",  "div",    "divu",   "dmult",   "dmultu", "ddiv",   "ddivu",
        "add",    "addu",   "sub",    "subu",   "and",     "or",     "xor",    "nor",
        "s50err", "s51err", "slt",    "sltu",   "dadd",    "daddu",  "dsub",   "dsubu",
        "tge",    "tgeu",   "tlt",    "tltu",   "teq",     "s65err", "tne",    "s67err",
        "dsll",   "s71err", "dsrl",   "dsra",   "dsll32",  "s75err", "dsrl32", "dsra32",
    };
    enum class InstructionType {
        SPECIAL,
        REGIMM,
        J,
        JAL,
        BEQ,
        BNE,
        BLEZ,
        BGTZ,
        ADDI,
        ADDIU,
        SLTI,
        STLIU,
        ANDI,
        ORI,
        XORI,
        LUI,
        COP0,
        COP1,
        COP2,
        _23ERR,
        BEQL,
        BNEL,
        BLEZL,
        BGTZL,
        DADDI,
        DADDIU,
        LDL,
        LDR,
        _34ERR,
        _35ERR,
        _36ERR,
        _37ERR,
        LB,
        LH,
        LWL,
        LW,
        LBU,
        LHU,
        LWR,
        LWU,
        SB,
        SH,
        SWL,
        SW,
        SDL,
        SDR,
        SWR,
        CACHE,
        LL,
        LWC1,
        LWC2,
        _63ERR,
        LLD,
        LDC1,
        LDC2,
        LD,
        SC,
        SWC1,
        SWC2,
        _73ERR,
        SCD,
        SDC1,
        SDC2,
        SD,

        s_SLL,
        _S2ERR,
        s_SRL,
        s_SRA,
        s_SLLV,
        _S5ERR,
        s_SRLV,
        s_SRAV,
        s_JR,
        s_JALR,
        _S12ERR,
        _S13ERR,
        s_SYSCALL,
        s_BREAK,
        _S16ERR,
        s_SYNC,
        s_MFHI,
        s_MTHI,
        s_MFLO,
        s_MTLO,
        s_DSLLV,
        _S25ERR,
        s_DSRLV,
        s_DSRAV,
        s_MULT,
        s_MULTU,
        s_DIV,
        s_DIVU,
        s_DMULT,
        s_DMULTU,
        s_DDIV,
        s_DDIVU,
        s_ADD,
        s_ADDU,
        s_SUB,
        s_SUBU,
        s_AND,
        s_OR,
        s_XOR,
        s_NOR,
        _S50ERR,
        _S51ERR,
        s_SLT,
        s_SLTU,
        s_DADD,
        s_DADDU,
        s_DSUB,
        s_DSUBU,
        s_TGE,
        s_TGEU,
        s_TLT,
        s_TLTU,
        s_TEQ,
        _S65ERR,
        s_TNE,
        _S67ERR,
        s_DSLL,
        _S71ERR,
        s_DSRL,
        s_DSRA,
        s_DSLL32,
        _S75ERR,
        s_DSRL32,
        s_DSRA32,

        NOP,
        ERROR
    };

    [[maybe_unused]] static std::string gpr_get_name(int n, bool register_names)
    {
        if (register_names)
        {
            switch (n)
            {
                case 0:
                    return "zero";
                case 1:
                    return "at";
                case 2 ... 3:
                    return "v" + std::to_string(n - 2);
                case 4 ... 7:
                    return "a" + std::to_string(n - 4);
                case 8 ... 15:
                    return "t" + std::to_string(n - 8);
                case 16 ... 23:
                    return "s" + std::to_string(n - 16);
                case 24 ... 25:
                    return "t" + std::to_string(n - 16);
                case 26 ... 27:
                    return "k" + std::to_string(n - 26);
                case 28:
                    return "gp";
                case 29:
                    return "sp";
                case 30:
                    return "fp";
                case 31:
                    return "ra";
                default:
                    return "error";
            }
        }
        return "r" + std::to_string(n);
    }

    struct DisassemblerInstruction
    {
        uint32_t vaddr;
        std::string disassembly;
    };

    constexpr static uint32_t CPzOPERATION_BIT = 0b00000010'00000000'00000000'00000000;
    constexpr static uint32_t CPz_TO_GPR = 0b00010;
    using Instruction = InstructionBase;
    using GPRRegister = MemDataUnionDW;
    static_assert(std::endian::native == std::endian::little,
                  "This emulator does not work on big endian systems!");
    static_assert(sizeof(Instruction) == sizeof(uint32_t), "N64 instruction should be 4 bytes!");
    static_assert(sizeof(double) == 8, "double data type is not 8 bytes!");
    static_assert(sizeof(MemDataUnionDW) == sizeof(int64_t), "Size of MemDataUnionDW mismatch!");
    static_assert(std::numeric_limits<float>::is_iec559,
                  "float data type is not ISO/IEC/IEEE 60559:2011 compliant!");
} // namespace hydra::N64
