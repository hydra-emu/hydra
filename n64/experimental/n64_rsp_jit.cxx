#include <n64/core/n64_rsp_jit.hxx>

uint64_t se64(uint16_t imm)
{
    int16_t temp = imm;
    return static_cast<int64_t>(static_cast<int32_t>(temp));
}

uint32_t se32(uint16_t imm)
{
    int16_t temp = imm;
    return static_cast<int32_t>(temp);
}

namespace hydra::N64
{
    void RSPJit::create_block(uint32_t* code)
    {
        Xbyak::CodeGenerator cg;
        block_start(cg);
        // addi
        compile_instruction(cg, 0b001111'00000'00001'11111100'00000100);
        compile_instruction(cg, 0b001111'00000'00010'11111100'00000100);
        compile_instruction(cg, 0b001111'00000'00011'11111100'00000100);
        compile_instruction(cg, 0b001111'00000'00100'11111100'00000100);
        compile_instruction(cg, 0b001111'00000'00101'11111100'00000100);
        compile_instruction(cg, 0b001111'00000'00110'11111100'00000100);
        compile_instruction(cg, 0b001111'00000'00111'11111100'00000100);
        compile_instruction(cg, 0b001111'00000'01000'11111100'00000100);
        compile_instruction(cg, 0b001111'00000'01001'11111100'00000100);
        compile_instruction(cg, 0b001111'00000'01010'11111100'00000100);
        compile_instruction(cg, 0b001111'00000'01011'11111100'00000100);
        block_end(cg);
        // print code
        auto buf = cg.getCode();
        for (int i = 0; i < cg.getSize(); i++)
        {
            printf("%02x", buf[i]);
        }
    }

    void RSPJit::block_start(Xbyak::CodeGenerator& cg)
    {
        cg.mov(RegisterPointer, (uintptr_t)&registers_);
        cg.mov(PcPointer, (uintptr_t)&rpc_);
        for (int i = 0; i < 4; i++)
        {
            cg.push(NonVolatiles[i]);
        }
    }

    bool RSPJit::compile_instruction(Xbyak::CodeGenerator& cg, uint32_t instruction)
    {
        Instruction instr = {.full = instruction};
        using namespace hydra::N64;
        switch (static_cast<InstructionType>(instr.RType.op))
        {
            case InstructionType::ADDI:
            {
                uint64_t imm = se32(instr.IType.immediate);
                Reg64 rs = get_host_register(cg, instr.IType.rs);
                Reg64 rt = get_host_register(cg, instr.IType.rt);
                cg.add(rs, imm);
                if (rs != rt)
                {
                    cg.mov(rt, rs);
                }
                break;
            }
            case InstructionType::LUI:
            {
                uint32_t imm = se32(instr.IType.immediate) << 16;
                Reg64 rt = get_host_register(cg, instr.IType.rt);
                cg.mov(rt, imm);
                break;
            }
            case InstructionType::ORI:
            {
                uint64_t imm = instr.IType.immediate;
                Reg64 rs = get_host_register(cg, instr.IType.rs);
                Reg64 rt = get_host_register(cg, instr.IType.rt);
                cg.or_(rs, imm);
                if (rs != rt)
                {
                    cg.mov(rt, rs);
                }
                break;
            }
            default:
                return false;
        }
        return true;
    }

    void RSPJit::block_end(Xbyak::CodeGenerator& cg)
    {
        for (int i = 3; i >= 0; i--)
        {
            cg.pop(NonVolatiles[i]);
        }
        cg.ret();
    }

    Reg64 RSPJit::get_host_register(Xbyak::CodeGenerator& cg, uint8_t guest_reg)
    {
        auto host_reg_opt = register_allocations_[guest_reg];
        if (!host_reg_opt)
        {
            // Allocate a new host register
            Reg64 host_reg = allocateableRegisters[next_allocatable_host_register_];
            next_allocatable_host_register_++;
            if (next_allocatable_host_register_ == allocateableRegisters.size())
            {
                next_allocatable_host_register_ = 0;
            }
            for (int i = 0; i < 32; i++)
            {
                if (register_allocations_[i] == host_reg)
                {
                    register_allocations_[i] = std::nullopt;
                    // Flush old register
                    cg.mov(ptr[RegisterPointer + i * 8], host_reg);
                    break;
                }
            }
            // Load new register
            cg.mov(host_reg, ptr[RegisterPointer + guest_reg * 8]);
            register_allocations_[guest_reg] = host_reg;
            return host_reg;
        }
        else
        {
            return host_reg_opt.value();
        }
    }
} // namespace hydra::N64

int main()
{
    hydra::N64::RSPJit jit;
    jit.create_block(nullptr);
    return 0;
}