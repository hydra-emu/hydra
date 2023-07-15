#ifndef MIPS_JIT_HXX
#define MIPS_JIT_HXX

// Currently not used

#include <array>
#include <n64/core/n64_register_allocation.hxx>
#include <n64/core/n64_types.hxx>
#include <optional>
#include <vector>
#include <xbyak/xbyak.h>

namespace hydra::N64
{
    using block = int (*)();

    struct RSPJit
    {
        RSPJit() = default;
        void create_block(uint32_t* code);

    private:
        std::array<block, 0x400> blocks_;
        std::array<MemDataUnionDW, 32> registers_;
        uint64_t rpc_;

        void block_start(Xbyak::CodeGenerator& cg);
        bool compile_instruction(Xbyak::CodeGenerator& cg, uint32_t instruction);
        void block_end(Xbyak::CodeGenerator& cg);
        Reg64 get_host_register(Xbyak::CodeGenerator& cg, uint8_t guest_reg);

        std::array<std::optional<Reg64>, 32> register_allocations_;
        int next_allocatable_host_register_ = 0;
    };
} // namespace hydra::N64
#endif