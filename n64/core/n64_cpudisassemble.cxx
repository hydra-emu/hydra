#include "n64_cpu.hxx"
#include <fmt/format.h>

namespace hydra::N64
{
    std::string disassemble_instr_special(uint32_t instr_num, bool register_names)
    {
        Instruction instr = {.full = instr_num};
        int char_count = SpecialCodes[instr.RType.func].size();
        std::string ret = SpecialCodes[instr.RType.func];
        ret += std::string(8 - char_count, ' ');
        switch (static_cast<InstructionType>(instr.RType.func + 64))
        {
            case InstructionType::s_JALR:
            {
                if (instr.RType.rd != 31)
                {
                    ret += fmt::format("{}, ", gpr_get_name(instr.RType.rd, register_names));
                }
                ret += fmt::format("{}", gpr_get_name(instr.RType.rs, register_names));
                break;
            }
            case InstructionType::s_SLL:
            {
                if (instr.full == 0)
                {
                    return "nop";
                }
                ret += fmt::format("{}, {}, {}", gpr_get_name(instr.RType.rd, register_names),
                                   gpr_get_name(instr.RType.rt, register_names),
                                   static_cast<uint8_t>(instr.RType.sa));
                break;
            }
            case InstructionType::s_JR:
            {
                ret += fmt::format("{}", gpr_get_name(instr.RType.rs, register_names));
                break;
            }
            case InstructionType::s_ADDU:
            {
                ret += fmt::format("{}, {}, {}", gpr_get_name(instr.RType.rd, register_names),
                                   gpr_get_name(instr.RType.rs, register_names),
                                   gpr_get_name(instr.RType.rt, register_names));
                break;
            }
        }
        return ret;
    }

    std::string disassemble_instr(uint32_t instr_num, bool register_names)
    {
        Instruction instr = {.full = instr_num};
        if (instr.IType.op == 0)
        {
            return disassemble_instr_special(instr_num, register_names);
        }
        int char_count = OperationCodes[instr.IType.op].size();
        std::string ret = OperationCodes[instr.IType.op];
        ret += std::string(8 - char_count, ' ');
        switch (static_cast<InstructionType>(instr.IType.op))
        {
            case InstructionType::J:
            case InstructionType::JAL:
            {
                ret += fmt::format("0x{:08x}", instr.JType.target << 2);
                break;
            }
            case InstructionType::ADDIU:
            case InstructionType::ORI:
            case InstructionType::ANDI:
            {
                ret += fmt::format("{}, {}, 0x{:04x}", gpr_get_name(instr.IType.rt, register_names),
                                   gpr_get_name(instr.IType.rs, register_names),
                                   static_cast<int16_t>(instr.IType.immediate));
                break;
            }
            case InstructionType::BEQ:
            case InstructionType::BNE:
            {
                ret += fmt::format("{}, {}, 0x{:04x}", gpr_get_name(instr.IType.rs, register_names),
                                   gpr_get_name(instr.IType.rt, register_names),
                                   static_cast<uint16_t>(instr.IType.immediate));
                break;
            }
            case InstructionType::BGTZ:
            case InstructionType::BLEZ:
            {
                ret += fmt::format("{}, 0x{:04x}", gpr_get_name(instr.IType.rs, register_names),
                                   static_cast<uint16_t>(instr.IType.immediate));
                break;
            }
            case InstructionType::LUI:
            {
                ret += fmt::format("{}, 0x{:04x}", gpr_get_name(instr.IType.rt, register_names),
                                   static_cast<uint16_t>(instr.IType.immediate));
                break;
            }
            case InstructionType::SD:
            case InstructionType::SW:
            case InstructionType::SH:
            case InstructionType::SB:
            case InstructionType::LD:
            case InstructionType::LW:
            case InstructionType::LWU:
            case InstructionType::LWL:
            case InstructionType::LWR:
            case InstructionType::LH:
            case InstructionType::LHU:
            case InstructionType::LB:
            case InstructionType::LBU:
            {
                if (instr.IType.immediate == 0)
                {
                    ret += fmt::format("{}, ({})", gpr_get_name(instr.IType.rt, register_names),
                                       gpr_get_name(instr.IType.rs, register_names));
                }
                else
                {
                    ret += fmt::format("{}, 0x{:04x}({})",
                                       gpr_get_name(instr.IType.rt, register_names),
                                       static_cast<uint16_t>(instr.IType.immediate),
                                       gpr_get_name(instr.IType.rs, register_names));
                }
                break;
            }
        }
        return ret;
    }

    std::vector<DisassemblerInstruction> CPU::disassemble(uint64_t start_vaddr, uint64_t end_vaddr,
                                                          bool register_names)
    {
        if (end_vaddr < start_vaddr)
        {
            throw std::runtime_error("End address is less than start address");
        }
        std::vector<DisassemblerInstruction> ret;
        int j = 0;
        int count = (end_vaddr - start_vaddr) >> 2;
        ret.resize(count);
        for (uint64_t i = start_vaddr; i < end_vaddr; i += 4)
        {
            uint32_t instr = __builtin_bswap32(
                *reinterpret_cast<uint32_t*>(cpubus_.redirect_paddress(translate_vaddr(i).paddr)));
            std::string instr_str = disassemble_instr(instr, register_names);
            DisassemblerInstruction instr_s = {.vaddr = static_cast<uint32_t>(i),
                                               .disassembly = instr_str};
            ret[j++] = instr_s;
        }
        return ret;
    }
} // namespace hydra::N64