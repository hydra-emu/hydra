#include "n64_cpu.hxx"

namespace hydra::N64
{

    void CPU::throw_exception(uint32_t address, ExceptionType type, uint8_t processor)
    {
        if (!CP0Status.EXL)
        {
            int64_t new_pc = static_cast<int32_t>(address);
            if (prev_branch_)
            {
                new_pc -= 4;
            }
            CP0Cause.BD = prev_branch_;
            cp0_regs_[CP0_EPC].UD = new_pc;
        } else
        {
            Logger::Warn("Nested exception");
            exit(1);
        }
        CP0Cause.ExCode = static_cast<uint8_t>(type);
        CP0Cause.CE = processor;
        CP0Status.EXL = true;
        switch (type)
        {
            case ExceptionType::FloatingPoint:
                Logger::Warn("Floating point exception {:08x}\n", instruction_.full);
                [[fallthrough]];
            case ExceptionType::Trap:
            case ExceptionType::Syscall:
            case ExceptionType::Interrupt:
            case ExceptionType::Breakpoint:
            case ExceptionType::TLBMissLoad:
            case ExceptionType::IntegerOverflow:
            case ExceptionType::AddressErrorLoad:
            case ExceptionType::AddressErrorStore:
            case ExceptionType::ReservedInstruction:
            case ExceptionType::CoprocessorUnusable:
            {
                pc_ = 0x8000'0180;
                next_pc_ = pc_ + 4;
                break;
            }
            default:
            {
                Logger::Fatal("Unhandled exception type: {}", static_cast<int>(type));
            }
        }
    }

} // namespace hydra::N64
