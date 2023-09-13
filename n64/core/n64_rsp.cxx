#include <compatibility.hxx>
#include <fmt/format.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <log.hxx>
#include <n64/core/n64_addresses.hxx>
#include <n64/core/n64_rdp.hxx>
#include <n64/core/n64_rsp.hxx>
#include <sstream>

namespace hydra::N64
{
    template <>
    void RSP::log_cpu_state<false>(bool, uint64_t)
    {
    }

    template <>
    void RSP::log_cpu_state<true>(bool use_crc, uint64_t instructions)
    {
        static uint64_t count = 0;
        count++;
        if (count >= instructions)
        {
            exit(1);
        }
        // Get crc32 of gpr and fpr regs
        if (use_crc)
        {
            uint32_t gprcrc = 0xFFFF'FFFF;
            uint32_t veccrc = 0xFFFF'FFFF;
            // uint32_t memcrc = 0xFFFF'FFFF;
            for (int i = 0; i < 32; i++)
            {
                gprcrc = hydra::crc32_u64(gprcrc, gpr_regs_[i].UW);
                for (int j = 0; j < 8; j++)
                {
                    veccrc = hydra::crc32_u16(veccrc, vu_regs_[i][j]);
                }
            }
            // for (int i = 0; i < 0x1000; i += 4)
            // {
            //     memcrc = hydra::crc32_u32(memcrc, hydra::bswap32(load_word(i)));
            // }
            // memcrc ^= 0xFFFF'FFFF;
            gprcrc ^= 0xFFFF'FFFF;
            veccrc ^= 0xFFFF'FFFF;
            printf("RSP: %08x %08x %08x", pc_, instruction_.full, gprcrc);
        }
        else
        {
            printf("%08x %08x", pc_, instruction_.full);
            for (int i = 1; i < 32; i++)
            {
                printf(" %08x", gpr_regs_[i].UW);
            }
        }
        printf("\n");
    }

    RSP::RSP()
    {
        status_.halt = true;
    }

    void RSP::Reset()
    {
        status_.full = 0;
        status_.halt = true;
        mem_.fill(0);
        std::for_each(gpr_regs_.begin(), gpr_regs_.end(), [](auto& reg) { reg.UW = 0; });
        std::for_each(vu_regs_.begin(), vu_regs_.end(), [](auto& reg) { reg.fill(0); });
        std::for_each(accumulator_.begin(), accumulator_.end(), [](auto& reg) { reg.Set(0); });
        vco_.Clear();
        vce_.Clear();
        vcc_.Clear();
        div_in_ = 0;
        div_out_ = 0;
        div_in_ready_ = false;
        instruction_.full = 0;
        mem_addr_ = 0;
        dma_imem_ = false;
        rdram_addr_ = 0;
        rd_len_ = 0;
        wr_len_ = 0;
        pc_ = 0;
        next_pc_ = 4;
        semaphore_ = false;
    }

    void RSP::Tick()
    {
        gpr_regs_[0].UW = 0;
        auto instruction = fetch_instruction();
        instruction_.full = instruction;

        log_cpu_state<RSP_LOGGING>(true, 10000000);

        pc_ = next_pc_ & 0xFFF;
        next_pc_ = (pc_ + 4) & 0xFFF;
        execute_instruction();
    }

    void RSP::execute_instruction()
    {
        (instruction_table_[instruction_.IType.op])(this);
    }

    uint32_t RSP::fetch_instruction()
    {
        uint32_t instruction = *reinterpret_cast<uint32_t*>(&mem_[0x1000 + (pc_ & 0xFFF)]);
        return hydra::bswap32(instruction);
    }

    uint8_t RSP::load_byte(uint16_t address)
    {
        return mem_[address & 0xFFF];
    }

    uint16_t RSP::load_halfword(uint16_t address)
    {
        uint16_t data = mem_[address & 0xFFF] | (mem_[(address + 1) & 0xFFF] << 8);
        return hydra::bswap16(data);
    }

    uint32_t RSP::load_word(uint16_t address)
    {
        uint32_t data = mem_[address & 0xFFF] | (mem_[(address + 1) & 0xFFF] << 8) |
                        (mem_[(address + 2) & 0xFFF] << 16) | (mem_[(address + 3) & 0xFFF] << 24);
        return hydra::bswap32(data);
    }

    void RSP::store_byte(uint16_t address, uint8_t data)
    {
        mem_[address & 0xFFF] = data;
    }

    void RSP::store_halfword(uint16_t address, uint16_t data)
    {
        data = hydra::bswap16(data);
        mem_[address & 0xFFF] = data & 0xFF;
        mem_[(address + 1) & 0xFFF] = (data >> 8) & 0xFF;
    }

    void RSP::store_word(uint16_t address, uint32_t data)
    {
        data = hydra::bswap32(data);
        mem_[address & 0xFFF] = data & 0xFF;
        mem_[(address + 1) & 0xFFF] = (data >> 8) & 0xFF;
        mem_[(address + 2) & 0xFFF] = (data >> 16) & 0xFF;
        mem_[(address + 3) & 0xFFF] = (data >> 24) & 0xFF;
    }

    void RSP::branch_to(uint16_t address)
    {
        // TODO: Make it so that addresses are always correct (i.e. don't & on EVERY tick)
        next_pc_ = (address & ~0b11) & 0xFFF;
    }

    void RSP::conditional_branch(bool condition, uint16_t address)
    {
        if (condition)
        {
            branch_to(address);
        }
    }

    void RSP::link_register(uint8_t reg)
    {
        gpr_regs_[reg].UW = pc_ + 4;
    }

    void RSP::read_dma()
    {
        auto bytes_per_row = (rd_len_ & 0xFFF) + 1;
        bytes_per_row = (bytes_per_row + 0x7) & ~0x7;
        uint32_t row_count = (rd_len_ >> 12) & 0xFF;
        uint32_t row_stride = (rd_len_ >> 20) & 0xFFF;
        auto rdram_index = rdram_addr_ & 0xFFFFF8;
        auto rsp_index = mem_addr_ & 0xFF8;
        uint8_t* dest = dma_imem_ ? &mem_[0x1000] : &mem_[0];
        uint8_t* source = rdram_ptr_;

        for (uint32_t i = 0; i < row_count + 1; i++)
        {
            for (uint32_t j = 0; j < bytes_per_row; j++)
            {
                dest[rsp_index++] = source[rdram_index++];
            }
            rdram_index += row_stride;
            rdram_index &= 0xFFFFF8;
            rsp_index &= 0xFF8;
        }

        mem_addr_ = rsp_index;
        mem_addr_ |= dma_imem_ ? 0x1000 : 0;
        rdram_addr_ = rdram_index;
        // After the DMA transfer is finished, this field contains the value 0xFF8
        // The reason is that the field is internally decremented by 8 for each transferred word
        // so the final value will be -8 (in hex, 0xFF8)
        rd_len_ = (row_stride << 20) | 0xFF8;

        if constexpr (RSP_LOGGING)
        {
            // printf("RSP: copied %d bytes\n", copy);
            // dump_mem();
        }
    }

    void RSP::write_dma()
    {
        auto bytes_per_row = (wr_len_ & 0xFFF) + 1;
        bytes_per_row = (bytes_per_row + 0x7) & ~0x7;
        uint32_t row_count = (wr_len_ >> 12) & 0xFF;
        uint32_t row_stride = (wr_len_ >> 20) & 0xFFF;
        auto rdram_index = rdram_addr_ & 0xFFFFF8;
        auto rsp_index = mem_addr_ & 0xFF8;
        uint8_t* dest = rdram_ptr_;
        uint8_t* source = dma_imem_ ? &mem_[0x1000] : &mem_[0];

        for (uint32_t i = 0; i < row_count + 1; i++)
        {
            for (uint32_t j = 0; j < bytes_per_row; j++)
            {
                dest[rdram_index++] = source[rsp_index++];
            }
            rdram_index += row_stride;
            rdram_index &= 0xFFFFF8;
            rsp_index &= 0xFF8;
        }

        mem_addr_ = rsp_index;
        mem_addr_ |= dma_imem_ ? 0x1000 : 0;
        rdram_addr_ = rdram_index;
        // After the DMA transfer is finished, this field contains the value 0xFF8
        // The reason is that the field is internally decremented by 8 for each transferred word
        // so the final value will be -8 (in hex, 0xFF8)
        wr_len_ = (row_stride << 20) | 0xFF8;
    }

    void RSP::dump_mem()
    {
        printf("rsp dma:\n");
        for (int i = 0; i < 0x2000; i += 4)
        {
            printf("%d: %02x %02x %02x %02x\n", i, mem_[i + 3], mem_[i + 2], mem_[i + 1],
                   mem_[i + 0]);
        }
        printf("\n");
    }

    void RSP::write_hwio(RSPHWIO addr, uint32_t data)
    {
        switch (addr)
        {
            case RSPHWIO::Cache:
            {
                mem_addr_ = data & 0b1111'1111'1111;
                dma_imem_ = data & 0b1'0000'0000'0000;
                break;
            }
            case RSPHWIO::DramAddr:
            {
                rdram_addr_ = data & 0b111'1111'1111'1111'1111'1111;
                break;
            }
            case RSPHWIO::RdLen:
            {
                rd_len_ = data;
                read_dma();
                break;
            }
            case RSPHWIO::WrLen:
            {
                wr_len_ = data;
                write_dma();
                break;
            }
            case RSPHWIO::Full:
            {
                status_.dma_full = data;
                break;
            }
            case RSPHWIO::Busy:
            {
                status_.dma_busy = data;
                break;
            }
            case RSPHWIO::Semaphore:
            {
                semaphore_ = false;
                break;
            }
            case RSPHWIO::Status:
            {
                RSPStatusWrite sp_write;
                sp_write.full = data;
                if (sp_write.clear_intr && !sp_write.set_intr)
                {
                    interrupt_callback_(false);
                }
                else if (!sp_write.clear_intr && sp_write.set_intr)
                {
                    Logger::Debug("Raising SP interrupt");
                    interrupt_callback_(true);
                }
                if (sp_write.clear_broke)
                {
                    status_.broke = false;
                }
#define flag(x)                                  \
    if (!sp_write.set_##x && sp_write.clear_##x) \
    {                                            \
        status_.x = false;                       \
    }                                            \
    if (sp_write.set_##x && !sp_write.clear_##x) \
    {                                            \
        status_.x = true;                        \
    }
                flag(signal_0);
                flag(signal_1);
                flag(signal_2);
                flag(signal_3);
                flag(signal_4);
                flag(signal_5);
                flag(signal_6);
                flag(signal_7);
                flag(halt);
                flag(intr_break);
                flag(sstep);
#undef flag
                break;
            }
            case RSPHWIO::CmdStart:
            {
                rdp_ptr_->WriteWord(DP_START, data);
                break;
            }
            case RSPHWIO::CmdEnd:
            {
                rdp_ptr_->WriteWord(DP_END, data);
                break;
            }
            case RSPHWIO::CmdStatus:
            {
                rdp_ptr_->WriteWord(DP_STATUS, data);
                break;
            }
            default:
            {
                Logger::Fatal("Unimplemented RSP HWIO write: {}", static_cast<int>(addr));
                break;
            }
        }
    }

    uint32_t RSP::read_hwio(RSPHWIO addr)
    {
        switch (addr)
        {
            case RSPHWIO::Cache:
                return mem_addr_;
            case RSPHWIO::DramAddr:
                return rdram_addr_;
            case RSPHWIO::RdLen:
                return rd_len_;
            case RSPHWIO::WrLen:
                return wr_len_;
            case RSPHWIO::Status:
                return status_.full;
            case RSPHWIO::Full:
                return status_.dma_full;
            case RSPHWIO::Busy:
                return status_.dma_busy;
            case RSPHWIO::Semaphore:
            {
                bool value = semaphore_;
                semaphore_ = true;
                return value;
            }
            case RSPHWIO::CmdStart:
            {
                return rdp_ptr_->start_address_;
            }
            case RSPHWIO::CmdEnd:
            {
                return rdp_ptr_->end_address_;
            }
            case RSPHWIO::CmdCurrent:
            {
                return rdp_ptr_->current_address_;
            }
            case RSPHWIO::CmdStatus:
            {
                return rdp_ptr_->ReadWord(DP_STATUS);
            }
            case RSPHWIO::CmdClock:
            {
                Logger::Warn("Unimplemented RDP command clock read");
                return 0;
            }
            case RSPHWIO::CmdBusy:
            {
                Logger::Warn("Unimplemented RDP command busy read");
                return 0;
            }
            case RSPHWIO::CmdPipeBusy:
            {
                Logger::Warn("Unimplemented RDP command pipe busy read");
                return 0;
            }
            case RSPHWIO::CmdTmemBusy:
            {
                Logger::Warn("Unimplemented RDP command TMEM busy read");
                return 0;
            }
            default:
            {
                Logger::Fatal("Illegal RSP HWIO read: {}", static_cast<int>(addr));
                return 0;
            }
        }
    }

    bool RSP::IsHalted()
    {
        return status_.halt;
    }

    void RSP::InstallBuses(uint8_t* rdram_ptr, RDP* rdp_ptr)
    {
        rdram_ptr_ = rdram_ptr;
        rdp_ptr_ = rdp_ptr;
    }

    void RSP::SetInterruptCallback(std::function<void(bool)> callback)
    {
        interrupt_callback_ = callback;
    }
} // namespace hydra::N64