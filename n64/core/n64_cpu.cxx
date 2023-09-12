#include "n64/core/n64_addresses.hxx"
#include <bitset>
#include <cassert>
#include <cmath>
#include <compatibility.hxx>
#include <cstring>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <n64/core/n64_cpu.hxx>
#include <random>
#include <sstream>

namespace hydra::N64
{
    std::vector<uint8_t> CPUBus::ipl_{};

    CPUBus::CPUBus(RCP& rcp) : rcp_(rcp)
    {
        cart_rom_.resize(0xFC00000);
        rdram_.resize(0x800000);
        map_direct_addresses();
    }

    bool CPUBus::LoadCartridge(std::string path)
    {
        std::ifstream ifs(path, std::ios::in | std::ios::binary);
        if (ifs.is_open())
        {
            ifs.unsetf(std::ios::skipws);
            ifs.seekg(0, std::ios::end);
            std::streampos size = ifs.tellg();
            ifs.seekg(0, std::ios::beg);
            ifs.read(reinterpret_cast<char*>(cart_rom_.data()), size);
            rom_loaded_ = true;
            Reset();
        }
        else
        {
            return false;
        }
        return true;
    }

    bool CPUBus::LoadIPL(std::string path)
    {
        std::ifstream ifs(path, std::ios::in | std::ios::binary);
        if (ifs.is_open())
        {
            if (CPUBus::ipl_.empty())
            {
                ifs.unsetf(std::ios::skipws);
                ifs.seekg(0, std::ios::end);
                std::streampos size = ifs.tellg();
                ifs.seekg(0, std::ios::beg);
                CPUBus::ipl_.resize(size);
                ifs.read(reinterpret_cast<char*>(CPUBus::ipl_.data()), size);
            }
        }
        else
        {
            return false;
        }
        ipl_loaded_ = !ipl_.empty();
        return true;
    }

    // These resets are called multiple times - why?
    void CPUBus::Reset()
    {
        pif_ram_.fill(0);
        time_ = 0;

        uint32_t crc = 0xFFFF'FFFF;
        for (int i = 0; i < 0x9c0; i++)
        {
            crc = hydra::crc32_u8(crc, cart_rom_[i + 0x40]);
        }
        crc ^= 0xFFFF'FFFF;

        switch (crc)
        {
            // CIC-NUS-6103
            case 0xea8f8526:
            {
                pif_ram_[0x26] = 0x78;
                break;
            }
            // CIC-NUS-6105
            case 0x1abca43c:
            {
                pif_ram_[0x26] = 0x91;
                break;
            }
            // CIC-NUS-6106
            case 0x7d286472:
            {
                pif_ram_[0x26] = 0x85;
                break;
            }
            default:
            {
                Logger::Warn("Unknown CIC: {:08X}", crc);
                pif_ram_[0x26] = 0x3F;
                break;
            }
        }

        pif_ram_[0x27] = 0x3F;
    }

    uint8_t* CPUBus::redirect_paddress(uint32_t paddr)
    {
        uint8_t* ptr = page_table_[paddr >> 16];
        if (ptr) [[likely]]
        {
            ptr += (paddr & static_cast<uint32_t>(0xFFFF));
            return ptr;
        }
        else if (paddr - 0x1FC00000u < 1984u)
        {
            return &ipl_[paddr - 0x1FC00000u];
        }
        return nullptr;
    }

    void CPUBus::map_direct_addresses()
    {
        // https://wheremyfoodat.github.io/software-fastmem/
        const uint32_t PAGE_SIZE = 0x10000;
#define ADDR_TO_PAGE(addr) ((addr) >> 16)
        // Map rdram
        for (int i = 0; i < ADDR_TO_PAGE(0x800'000); i++)
        {
            page_table_[i] = &rdram_[PAGE_SIZE * i];
        }
        page_table_[ADDR_TO_PAGE(0x04000000)] = &rcp_.rsp_.mem_[0];

        for (int i = ADDR_TO_PAGE(0x08000000); i <= ADDR_TO_PAGE(0x0FFF'0000); i++)
        {
            // page_table_[i] = &sram_[PAGE_SIZE * (i - ADDR_TO_PAGE(0x0800'0000))];
        }

        // Map cartridge rom
        for (int i = ADDR_TO_PAGE(0x1000'0000); i <= ADDR_TO_PAGE(0x1FBF'0000); i++)
        {
            page_table_[i] = &cart_rom_[PAGE_SIZE * (i - ADDR_TO_PAGE(0x1000'0000))];
        }
        page_table_[ADDR_TO_PAGE(ISVIEWER_AREA_START)] = nullptr;
#undef ADDR_TO_PAGE
    }

    template <>
    void CPU::log_cpu_state<false>(bool, uint64_t, uint64_t)
    {
    }

    template <>
    void CPU::log_cpu_state<true>(bool use_crc, uint64_t instructions, uint64_t start)
    {
        static uint64_t count = 0;
        count++;
        if (count >= start + instructions)
        {
            exit(1);
        }
        if (count < start)
        {
            return;
        }
        // Get crc32 of gpr and fpr regs
        if (use_crc)
        {
            uint32_t gprcrc = 0xFFFF'FFFF;
            uint32_t fprcrc = 0xFFFF'FFFF;
            // uint32_t pifcrc = 0xFFFF'FFFF;
            for (int i = 0; i < 32; i++)
            {
                gprcrc = hydra::crc32_u64(gprcrc, gpr_regs_[i].UD);
                fprcrc = hydra::crc32_u64(fprcrc, fpr_regs_[i].UD);
            }
            for (int i = 0; i < 64; i++)
            {
                // pifcrc = hydra::crc32_u8(pifcrc, cpubus_.pif_ram_[i]);
            }
            gprcrc ^= 0xFFFF'FFFF;
            fprcrc ^= 0xFFFF'FFFF;
            // pifcrc ^= 0xFFFF'FFFF;
            printf("%08lx %08x %08x %08x", pc_ & 0xFFFF'FFFF, instruction_.full, gprcrc, fprcrc);
        }
        else
        {
            printf("%08lx %08x ", pc_, instruction_.full);
            for (int i = 1; i < 32; i++)
            {
                printf("%016lx ", gpr_regs_[i].UD);
            }
        }
        printf("\n");
    }

    void CPU::set_interrupt(InterruptType type, bool value)
    {
        switch (type)
        {
            case InterruptType::VI:
            {
                cpubus_.mi_interrupt_.VI = value;
                break;
            }
            case InterruptType::SI:
            {
                cpubus_.mi_interrupt_.SI = value;
                break;
            }
            case InterruptType::AI:
            {
                cpubus_.mi_interrupt_.AI = value;
                break;
            }
            case InterruptType::PI:
            {
                cpubus_.mi_interrupt_.PI = value;
                break;
            }
            case InterruptType::DP:
            {
                cpubus_.mi_interrupt_.DP = value;
                break;
            }
            case InterruptType::SP:
            {
                cpubus_.mi_interrupt_.SP = value;
                break;
            }
        }
        update_interrupt_check();
    }

    void CPU::write_hwio(uint32_t addr, uint32_t data)
    {
        // TODO: remove switch, turn into if chain
        switch (addr)
        {
            case PI_STATUS:
            {
                if (data & 0b10)
                {
                    set_interrupt(InterruptType::PI, false);
                }
                return;
            }
            case PI_DRAM_ADDR:
            {
                cpubus_.pi_dram_addr_ = data;
                return;
            }
            case PI_CART_ADDR:
            {
                cpubus_.pi_cart_addr_ = data;
                return;
            }
            case PI_RD_LEN:
            {
                // std::memcpy(&cpubus_.rdram_[hydra::bswap32(cpubus_.pi_cart_addr_)],
                // cpubus_.redirect_paddress(hydra::bswap32(cpubus_.pi_dram_addr_)), data + 1);
                Logger::Warn("PI_RD_LEN write");
                set_interrupt(InterruptType::PI, true);
                return;
            }
            case PI_WR_LEN:
            {
                auto cart_addr = cpubus_.pi_cart_addr_ & 0xFFFFFFFE;
                auto dram_addr = cpubus_.pi_dram_addr_ & 0x007FFFFE;
                uint64_t length = data + 1;
                if (cart_addr >= 0x8000000 && cart_addr < 0x10000000)
                {
                    Logger::Warn("DMA to SRAM is unimplemented!");
                    cpubus_.dma_busy_ = false;
                    set_interrupt(InterruptType::PI, true);
                    return;
                }
                std::memcpy(&cpubus_.rdram_[dram_addr], cpubus_.redirect_paddress(cart_addr),
                            length);
                cpubus_.dma_busy_ = true;
                // uint8_t domain = 0;
                // if ((cart_addr >= 0x0800'0000 && cart_addr < 0x1000'0000) ||
                //     (cart_addr >= 0x0500'0000 && cart_addr < 0x0600'0000))
                // {
                //     domain = 2;
                // } else if ((cart_addr >= 0x0600'0000 && cart_addr < 0x0800'0000) ||
                //            (cart_addr >= 0x1000'0000 && cart_addr < 0x1FC0'0000))
                // {
                //     domain = 1;
                // }
                // auto cycles = timing_pi_access(domain, length);
                cpubus_.dma_busy_ = false;
                set_interrupt(InterruptType::PI, true);
                Logger::Debug("Raising PI interrupt");
                return;
            }
            case PI_BSD_DOM1_PWD:
            {
                cpubus_.pi_bsd_dom1_pwd_ = data & 0xFF;
                return;
            }
            case PI_BSD_DOM2_PWD:
            {
                cpubus_.pi_bsd_dom2_lat_ = data & 0xFF;
                return;
            }
            case PI_BSD_DOM1_PGS:
            {
                cpubus_.pi_bsd_dom1_pgs_ = data & 0xFF;
                return;
            }
            case PI_BSD_DOM2_PGS:
            {
                cpubus_.pi_bsd_dom2_pgs_ = data & 0xFF;
                return;
            }
            case PI_BSD_DOM1_LAT:
            {
                cpubus_.pi_bsd_dom1_lat_ = data & 0xFF;
                return;
            }
            case PI_BSD_DOM2_LAT:
            {
                cpubus_.pi_bsd_dom2_lat_ = data & 0xFF;
                return;
            }
            case PI_BSD_DOM1_RLS:
            {
                cpubus_.pi_bsd_dom1_rls_ = data & 0xFF;
                return;
            }
            case PI_BSD_DOM2_RLS:
            {
                cpubus_.pi_bsd_dom2_rls_ = data & 0xFF;
                return;
            }
            case MI_MODE:
            {
                // TODO: properly implement
                cpubus_.mi_mode_ = data;

                if ((data >> 11) & 0b1)
                {
                    set_interrupt(InterruptType::DP, false);
                }
                return;
            }
            case MI_MASK:
            {
                for (int j = 2, i = 0; i < 6; i++)
                {
                    if (data & j)
                    {
                        cpubus_.mi_mask_ |= 1 << i;
                    }
                    j <<= 2;
                }
                for (int j = 1, i = 0; i < 6; i++)
                {
                    if (data & j)
                    {
                        cpubus_.mi_mask_ &= ~(1 << i);
                    }
                    j <<= 2;
                }
                update_interrupt_check();
                return;
            }
            case SI_DRAM_ADDR:
            {
                cpubus_.si_dram_addr_ = data;
                return;
            }
            case SI_PIF_AD_WR64B:
            {
                std::memcpy(cpubus_.pif_ram_.data(),
                            &cpubus_.rdram_[cpubus_.si_dram_addr_ & 0xff'ffff], 64);
                pif_command();
                set_interrupt(InterruptType::SI, true);
                Logger::Debug("Raising SI interrupt");
                return;
            }
            case SI_PIF_AD_RD64B:
            {
                pif_command();
                std::memcpy(&cpubus_.rdram_[cpubus_.si_dram_addr_ & 0xff'ffff],
                            cpubus_.pif_ram_.data(), 64);
                set_interrupt(InterruptType::SI, true);
                Logger::Debug("Raising SI interrupt");
                return;
            }
            case SI_STATUS:
            {
                set_interrupt(InterruptType::SI, false);
                return;
            }
        }
        // Video interface
        if (addr >= VI_AREA_START && addr <= VI_AREA_END)
        {
            rcp_.vi_.WriteWord(addr, data);
        }
        // Audio interface
        else if (addr >= AI_AREA_START && addr <= AI_AREA_END)
        {
            rcp_.ai_.WriteWord(addr, data);
        }
        else if (addr >= RSP_AREA_START && addr <= RSP_AREA_END)
        {
            switch (addr)
            {
                case RSP_DMA_SPADDR:
                    rcp_.rsp_.write_hwio(RSPHWIO::Cache, data);
                    break;
                case RSP_DMA_RAMADDR:
                    rcp_.rsp_.write_hwio(RSPHWIO::DramAddr, data);
                    break;
                case RSP_DMA_RDLEN:
                    rcp_.rsp_.write_hwio(RSPHWIO::RdLen, data);
                    break;
                case RSP_DMA_WRLEN:
                    rcp_.rsp_.write_hwio(RSPHWIO::WrLen, data);
                    break;
                case RSP_STATUS:
                    rcp_.rsp_.write_hwio(RSPHWIO::Status, data);
                    break;
                case RSP_SEMAPHORE:
                    rcp_.rsp_.write_hwio(RSPHWIO::Semaphore, data);
                    break;
                case RSP_PC:
                {
                    if (!rcp_.rsp_.status_.halt)
                    {
                        Logger::Warn("RSP PC write while not halted");
                    }
                    rcp_.rsp_.pc_ = data & 0xffc;
                    rcp_.rsp_.next_pc_ = rcp_.rsp_.pc_ + 4;
                    return;
                }
            }
        }
        else if (addr >= RDP_AREA_START && addr <= RDP_AREA_END)
        {
            rcp_.rdp_.WriteWord(addr, data);
        }
        else if (addr >= PIF_START && addr <= PIF_END)
        {
            uint8_t* pif_ptr = reinterpret_cast<uint8_t*>(&cpubus_.pif_ram_[addr - PIF_START]);
            uint32_t swapped = hydra::bswap32(data);
            memcpy(pif_ptr, &swapped, 4);
            pif_command();
        }
        else if (addr == PIF_COMMAND)
        {
            cpubus_.pif_ram_[63] = data;
            pif_command();
        }
        else if (addr == ISVIEWER_FLUSH)
        {
            std::stringstream ss;
            for (uint32_t i = 0; i < data; i++)
            {
                ss << cpubus_.isviewer_buffer_[i];
            }
            std::cout << ss.str();
        }
        else if (addr >= ISVIEWER_AREA_START && addr <= ISVIEWER_AREA_END)
        {
            data = hydra::bswap32(data);
            for (int i = 0; i < 4; i++)
            {
                cpubus_.isviewer_buffer_[addr - ISVIEWER_AREA_START + i] = data >> (i * 8);
            }
        }
        else if (addr >= RI_AREA_START && addr <= RI_AREA_END)
        {
            Logger::Warn("Write to RI register {:x} with data {:x}", addr, data);
        }
        else if (addr >= RDRAM_REGISTERS_START && addr <= RDRAM_REGISTERS_END)
        {
            Logger::Warn("Write to RDRAM register {:x} with data {:x}", addr, data);
        }
        else if (addr >= RDRAM_BROADCAST_START && addr <= RDRAM_BROADCAST_END)
        {
            Logger::Warn("Write to RDRAM broadcast register {:x} with data {:x}", addr, data);
        }
        else
        {
            Logger::Warn("Unhandled write_hwio to address: {:08x} {:08x}", addr, data);
        }
    }

    uint32_t CPU::read_hwio(uint32_t addr)
    {
#define redir_case(addr, data) \
    case addr:                 \
        return data;
        switch (addr)
        {
            // MIPS Interface
            redir_case(MI_MODE, cpubus_.mi_mode_);
            case MI_VERSION:
            {
                return 0x02020102;
            }
                redir_case(MI_INTERRUPT, cpubus_.mi_interrupt_.full);
                redir_case(MI_MASK, cpubus_.mi_mask_);
                // Peripheral Interface
                redir_case(PI_DRAM_ADDR, cpubus_.pi_dram_addr_);
                redir_case(PI_CART_ADDR, cpubus_.pi_cart_addr_);
                redir_case(PI_RD_LEN, cpubus_.pi_rd_len_);
                redir_case(PI_WR_LEN, cpubus_.pi_wr_len_);
            case PI_STATUS:
            {
                return cpubus_.dma_busy_ | (cpubus_.io_busy_ << 1) | (cpubus_.dma_error_ << 2) |
                       (cpubus_.mi_interrupt_.PI << 3);
            }
                redir_case(PI_BSD_DOM1_LAT, cpubus_.pi_bsd_dom1_lat_);
                redir_case(PI_BSD_DOM1_PWD, cpubus_.pi_bsd_dom1_pwd_);
                redir_case(PI_BSD_DOM1_PGS, cpubus_.pi_bsd_dom1_pgs_);
                redir_case(PI_BSD_DOM1_RLS, cpubus_.pi_bsd_dom1_rls_);
                redir_case(PI_BSD_DOM2_LAT, cpubus_.pi_bsd_dom2_lat_);
                redir_case(PI_BSD_DOM2_PWD, cpubus_.pi_bsd_dom2_pwd_);
                redir_case(PI_BSD_DOM2_PGS, cpubus_.pi_bsd_dom2_pgs_);
                redir_case(PI_BSD_DOM2_RLS, cpubus_.pi_bsd_dom2_rls_);

                // RDRAM Interface
                redir_case(RI_MODE, cpubus_.ri_mode_);
                redir_case(RI_CONFIG, cpubus_.ri_config_);
                redir_case(RI_CURRENT_LOAD, cpubus_.ri_current_load_);
                redir_case(RI_SELECT, 0x14); // TODO: implement
                redir_case(RI_REFRESH, cpubus_.ri_refresh_);
                redir_case(RI_LATENCY, cpubus_.ri_latency_);

                // Serial Interface
                redir_case(SI_DRAM_ADDR, cpubus_.si_dram_addr_);
                redir_case(SI_PIF_AD_WR64B, cpubus_.si_pif_ad_wr64b_);
                redir_case(SI_PIF_AD_RD64B, cpubus_.si_pif_ad_rd64b_);
                redir_case(SI_STATUS, cpubus_.si_status_);

            // RSP registers
            case RSP_DMA_SPADDR:
                return rcp_.rsp_.read_hwio(RSPHWIO::Cache);
            case RSP_DMA_RAMADDR:
                return rcp_.rsp_.read_hwio(RSPHWIO::DramAddr);
            case RSP_DMA_RDLEN:
                return rcp_.rsp_.read_hwio(RSPHWIO::RdLen);
            case RSP_DMA_WRLEN:
                return rcp_.rsp_.read_hwio(RSPHWIO::WrLen);
            case RSP_STATUS:
                return rcp_.rsp_.read_hwio(RSPHWIO::Status);
            case RSP_DMA_FULL:
                return rcp_.rsp_.read_hwio(RSPHWIO::Full);
            case RSP_DMA_BUSY:
                return rcp_.rsp_.read_hwio(RSPHWIO::Busy);
            case RSP_SEMAPHORE:
                return rcp_.rsp_.read_hwio(RSPHWIO::Semaphore);
                redir_case(PIF_COMMAND, cpubus_.pif_ram_[63]);
            case RSP_PC:
            {
                if (!rcp_.rsp_.status_.halt)
                {
                    Logger::Warn("Reading from RSP_PC while not halted");
                }
                return rcp_.rsp_.pc_;
            }
            case ISVIEWER_FLUSH:
            {
                Logger::Fatal("Reading from ISViewer");
                return 0;
            }
            default:
            {
                break;
            }
        }
#undef redir_case
        // Video interface
        if (addr >= VI_AREA_START && addr <= VI_AREA_END)
        {
            return rcp_.vi_.ReadWord(addr);
        }
        // Audio Interface
        else if (addr >= AI_AREA_START && addr <= AI_AREA_END)
        {
            return rcp_.ai_.ReadWord(addr);
        }
        else if (addr >= PIF_START && addr <= PIF_END)
        {
            uint8_t* pif_ram = &cpubus_.pif_ram_[addr - PIF_START];
            uint32_t data = pif_ram[0] << 24 | pif_ram[1] << 16 | pif_ram[2] << 8 | pif_ram[3];
            return data;
        }
        else if (addr >= RDRAM_REGISTERS_START && addr <= RDRAM_REGISTERS_END)
        {
            Logger::Warn("Reading from RDRAM registers");
            return 0;
        }
        else if (addr >= ISVIEWER_AREA_START && addr <= ISVIEWER_AREA_END)
        {
            uint8_t* isviewer_ptr =
                reinterpret_cast<uint8_t*>(&cpubus_.isviewer_buffer_[addr - ISVIEWER_AREA_START]);
            uint32_t data = isviewer_ptr[0] << 24 | isviewer_ptr[1] << 16 | isviewer_ptr[2] << 8 |
                            isviewer_ptr[3];
            return data;
        }
        else if (addr >= RDP_AREA_START && addr <= RDP_AREA_END)
        {
            return rcp_.rdp_.ReadWord(addr);
        }
        else if (addr >= N64DD_AREA_START && addr <= N64DD_AREA_END)
        {
            Logger::Warn("Accessing N64DD");
            return 0;
        }
        else if (addr >= SRAM_AREA_START && addr <= SRAM_AREA_END)
        {
            Logger::Warn("Accessing SRAM");
            return 0;
        }
        Logger::Warn("Unhandled read_hwio from address {:08x} PC: {:08x}", addr, pc_);
        return 0;
    }

    enum class JoybusCommand : uint8_t
    {
        RequestInfo = 0,
        ControllerState = 1,
        ReadMempack = 2,
        WriteMempack = 3,
        ReadEEPROM = 4,
        WriteEEPROM = 5,
        Reset = 255,
    };

    void CPU::pif_command()
    {
        using namespace hydra::N64;
        auto command_byte = cpubus_.pif_ram_[63];
        if (command_byte & 0x1)
        {
            pif_channel_ = 0;
            int i = 0;
            while (i < 63)
            {
                int8_t tx = cpubus_.pif_ram_[i++];
                if (tx > 0)
                {
                    uint8_t* rx_ptr = &cpubus_.pif_ram_[i++];
                    if (*rx_ptr == 0xFE)
                    {
                        break;
                    }
                    uint8_t rx = *rx_ptr & 0x3F;
                    std::vector<uint8_t> data;
                    // get tx next bytes and send to pif chip
                    data.resize(tx);
                    for (int8_t j = 0; j < tx; j++)
                    {
                        data[j] = cpubus_.pif_ram_[i++];
                    }
                    // get response bytes
                    std::vector<uint8_t> response;
                    response.resize(rx);
                    if (joybus_command(data, response))
                    {
                        // Device not found
                        rx_ptr[0] |= 0x80;
                    }
                    for (size_t j = 0; j < response.size(); j++)
                    {
                        cpubus_.pif_ram_[i++] = response[j];
                    }
                    pif_channel_++;
                }
                else if (tx == 0)
                {
                    pif_channel_++;
                    continue;
                }
                else if (static_cast<uint8_t>(tx) == 0xFE)
                {
                    break;
                }
            }
        }
        if (command_byte & 0x8)
        {
            command_byte &= ~8;
        }
        if (command_byte & 0x20)
        {
            // cpubus_.pif_ram_[0x32] = 0;
            // cpubus_.pif_ram_[0x33] = 0;
            // cpubus_.pif_ram_[0x34] = 0;
            // cpubus_.pif_ram_[0x35] = 0;
            // cpubus_.pif_ram_[0x36] = 0;
            // cpubus_.pif_ram_[0x37] = 0;
        }
        if (command_byte & 0x30)
        {
            command_byte = 0x80;
        }
        cpubus_.pif_ram_[63] = command_byte;
    }

    void CPU::update_interrupt_check()
    {
        bool mi_interrupt = cpubus_.mi_interrupt_.full & cpubus_.mi_mask_;
        CP0Cause.IP2 = mi_interrupt;
        bool interrupts_pending = cp0_regs_[CP0_CAUSE].UB._1 & CP0Status.IM;
        bool interrupts_enabled = CP0Status.IE;
        bool currently_handling_exception = CP0Status.EXL;
        bool currently_handling_error = CP0Status.ERL;
        should_service_interrupt_ = interrupts_pending && interrupts_enabled &&
                                    !currently_handling_exception && !currently_handling_error;
    }

    bool CPU::joybus_command(const std::vector<uint8_t>& command, std::vector<uint8_t>& result)
    {
        if (result.size() == 0)
        {
            Logger::Fatal("Joybus command with no result");
        }
        JoybusCommand command_type = static_cast<JoybusCommand>(command[0]);
        switch (command_type)
        {
            case JoybusCommand::Reset:
            case JoybusCommand::RequestInfo:
            {
                if (result.size() != 3)
                {
                    dump_pif_ram();
                    Logger::Fatal("Joybus RequestInfo command with result size {}", result.size());
                }
                switch (pif_channel_)
                {
                    case 0:
                    {
                        uint16_t controller = static_cast<uint16_t>(controller_type_);
                        result[0] = controller >> 8;
                        result[1] = controller & 0xFF;
                        result[2] = 0x01;
                        break;
                    }
                    case 4:
                    {
                        result[0] = 0x00;
                        result[1] = 0x80;
                        result[2] = 0x00;
                        break;
                    }
                }
                break;
            }
            case JoybusCommand::ControllerState:
            {
                if (result.size() != 4)
                {
                    Logger::Fatal("Joybus ControllerState command with result size {}",
                                  result.size());
                }
                if (pif_channel_ != 0)
                {
                    return true;
                }

                get_controller_state(result, controller_type_);
                break;
            }
            case JoybusCommand::WriteMempack:
            {
                if (result.size() != 1)
                {
                    Logger::Fatal("Joybus WriteMempack command with result size {}", result.size());
                }
                result[0] = 0x80;
                break;
            }
            default:
            {
                Logger::WarnOnce("Unhandled joybus command type {}",
                                 static_cast<uint8_t>(command_type));
            }
        }
        return false;
    }

    void CPU::get_controller_state(std::vector<uint8_t>& result, ControllerType controller)
    {
        switch (controller)
        {
            case ControllerType::Keyboard:
            {
                result[0] = key_state_[Keys::A] << 7 | key_state_[Keys::B] << 6 |
                            key_state_[Keys::Z] << 5 | key_state_[Keys::Start] << 4 |
                            key_state_[Keys::KeypadUp] << 3 | key_state_[Keys::KeypadDown] << 2 |
                            key_state_[Keys::KeypadLeft] << 1 | key_state_[Keys::KeypadRight];
                result[1] = 0 | 0 | key_state_[Keys::L] << 5 | key_state_[Keys::R] << 4 |
                            key_state_[Keys::CUp] << 3 | key_state_[Keys::CDown] << 2 |
                            key_state_[Keys::CLeft] << 1 | key_state_[Keys::CRight];
                int8_t x = 0, y = 0;
                if (key_state_[Keys::Up])
                {
                    y = 127;
                }
                else if (key_state_[Keys::Down])
                {
                    y = -127;
                }
                if (key_state_[Keys::Left])
                {
                    x = -127;
                }
                else if (key_state_[Keys::Right])
                {
                    x = 127;
                }
                result[2] = x;
                result[3] = y;
                break;
            }
            case ControllerType::Mouse:
            {
                result[0] = key_state_[Keys::A] << 7 | key_state_[Keys::B] << 6;
                result[1] = 0;
                result[2] = mouse_delta_x_ << 2;
                result[3] = mouse_delta_y_ << 2;

                mouse_delta_x_ = 0;
                mouse_delta_y_ = 0;
                break;
            }
        }
    }

    CPU::CPU(CPUBus& cpubus, RCP& rcp)
        : gpr_regs_{}, fpr_regs_{}, instr_cache_(KB(16)), data_cache_(KB(8)), cpubus_(cpubus),
          rcp_(rcp)
    {
        rcp_.ai_.InstallBuses(&cpubus_.rdram_[0]);
        rcp_.vi_.InstallBuses(&cpubus_.rdram_[0]);
        rcp_.rsp_.InstallBuses(&cpubus_.rdram_[0], &rcp_.rdp_);
        rcp_.rdp_.InstallBuses(&cpubus_.rdram_[0], &rcp_.rsp_.mem_[0]);
        rcp_.ai_.SetInterruptCallback(
            std::bind(&CPU::set_interrupt, this, InterruptType::AI, std::placeholders::_1));
        rcp_.vi_.SetInterruptCallback(
            std::bind(&CPU::set_interrupt, this, InterruptType::VI, std::placeholders::_1));
        rcp_.rsp_.SetInterruptCallback(
            std::bind(&CPU::set_interrupt, this, InterruptType::SP, std::placeholders::_1));
        rcp_.rdp_.SetInterruptCallback(
            std::bind(&CPU::set_interrupt, this, InterruptType::DP, std::placeholders::_1));
    }

    void CPU::Reset()
    {
        pc_ = 0xFFFF'FFFF'BFC0'0000;
        next_pc_ = pc_ + 4;
        should_service_interrupt_ = false;
        for (auto& reg : gpr_regs_)
        {
            reg.UD = 0;
        }
        for (auto& reg : fpr_regs_)
        {
            reg.UD = 0;
        }
        for (auto& reg : cp0_regs_)
        {
            reg.UD = 0;
        }
        cpubus_.Reset();
        CP0Status.full = 0x3400'0000;
        CP0Cause.full = 0xB000'007C;
        cp0_regs_[CP0_EPC].UD = 0xFFFF'FFFF'FFFF'FFFFu;
        cp0_regs_[CP0_ERROREPC].UD = 0xFFFF'FFFF'FFFF'FFFFu;
        cp0_regs_[CP0_CONFIG].UD = 0x7006'E463;
        cp0_regs_[CP0_PRID].UD = 0x0000'0B22;
        CP0Context.full = 0;
        CP0XContext.full = 0;
        CP0EntryHi.full = 0;
        for (auto& entry : tlb_)
        {
            TLBEntry newentry{};
            newentry.initialized = false;
            std::swap(entry, newentry);
        }
        store_word(
            0x8000'0318,
            0x800000); // TODO: probably done by pif somewhere if RI_SELECT is emulated or something
    }

    // Shamelessly stolen from dillon
    // Thanks m64p
    uint32_t CPU::timing_pi_access(uint8_t domain, uint32_t length)
    {
        uint32_t cycles = 0;
        uint32_t latency = 0;
        uint32_t pulse_width = 0;
        uint32_t release = 0;
        uint32_t page_size = 0;
        uint32_t pages = 0;
        switch (domain)
        {
            case 1:
                latency = cpubus_.pi_bsd_dom1_lat_ + 1;
                pulse_width = cpubus_.pi_bsd_dom1_pwd_ + 1;
                release = cpubus_.pi_bsd_dom1_rls_ + 1;
                page_size = std::pow(2, (cpubus_.pi_bsd_dom1_pgs_ + 2));
                break;
            case 2:
                latency = cpubus_.pi_bsd_dom2_lat_ + 1;
                pulse_width = cpubus_.pi_bsd_dom2_pwd_ + 1;
                release = cpubus_.pi_bsd_dom2_rls_ + 1;
                page_size = pow(2, (cpubus_.pi_bsd_dom2_pgs_ + 2));
                break;
            default:
                Logger::Fatal("Invalid PI domain");
        }
        pages = ceil((double)length / page_size);
        cycles += (14 + latency) * pages;
        cycles += (pulse_width + release) * (length / 2);
        cycles += 5 * pages;
        return cycles * 1.5; // Converting RCP clock speed to CPU clock speed
    }

    TranslatedAddress CPU::translate_vaddr(uint32_t addr)
    {
        if (is_kernel_mode()) [[likely]]
        {
            return translate_vaddr_kernel(addr);
        }
        else
        {
            Logger::Fatal("Non kernel mode :(");
        }
        return {};
    }

    TranslatedAddress CPU::translate_vaddr_kernel(uint32_t addr)
    {
        if (addr >= 0x80000000 && addr <= 0xBFFFFFFF) [[likely]]
        {
            return {addr & 0x1FFFFFFF, true, true};
        }
        else if (addr >= 0 && addr <= 0x7FFFFFFF)
        {
            // User segment
            TranslatedAddress paddr = probe_tlb(addr);
            if (!paddr.success)
            {
                throw_exception(prev_pc_, ExceptionType::TLBMissLoad);
                set_cp0_regs_exception(addr);
            }
            return paddr;
        }
        else if (addr >= 0xC0000000 && addr <= 0xDFFFFFFF)
        {
            // Supervisor segment
            Logger::Warn("Accessing supervisor segment {:08x}", addr);
        }
        else
        {
            // Kernel segment TLB
            Logger::Warn("Accessing kernel segment {:08x}", addr);
        }
        return {};
    }

    uint8_t CPU::load_byte(uint64_t vaddr)
    {
        TranslatedAddress paddr = translate_vaddr(vaddr);
        uint8_t* ptr = cpubus_.redirect_paddress(paddr.paddr);

        if (!ptr)
        {
            Logger::Fatal("Attempted to load byte from invalid address: {:08x}", vaddr);
        }

        return *ptr;
    }

    uint16_t CPU::load_halfword(uint64_t vaddr)
    {
        TranslatedAddress paddr = translate_vaddr(vaddr);
        uint16_t* ptr = reinterpret_cast<uint16_t*>(cpubus_.redirect_paddress(paddr.paddr));

        if (!ptr)
        {
            Logger::Fatal("Attempted to load halfword from invalid address: {:08x}", vaddr);
        }

        uint16_t data;
        memcpy(&data, ptr, sizeof(uint16_t));
        return hydra::bswap16(data);
    }

    uint32_t CPU::load_word(uint64_t vaddr)
    {
        TranslatedAddress paddr = translate_vaddr(vaddr);
        uint8_t* ptr = cpubus_.redirect_paddress(paddr.paddr);
        if (!ptr)
        {
            return read_hwio(paddr.paddr);
        }
        else
        {
            uint32_t data;
            memcpy(&data, ptr, sizeof(uint32_t));
            return hydra::bswap32(data);
        }
    }

    uint64_t CPU::load_doubleword(uint64_t vaddr)
    {
        TranslatedAddress paddr = translate_vaddr(vaddr);
        uint64_t* ptr = reinterpret_cast<uint64_t*>(cpubus_.redirect_paddress(paddr.paddr));

        if (!ptr)
        {
            Logger::Fatal("Attempted to load doubleword from invalid address: {:08x}", vaddr);
        }

        uint64_t data;
        memcpy(&data, ptr, sizeof(uint64_t));
        return hydra::bswap64(data);
    }

    void CPU::store_byte(uint64_t vaddr, uint8_t data)
    {
        TranslatedAddress paddr = translate_vaddr(vaddr);
        uint8_t* ptr = cpubus_.redirect_paddress(paddr.paddr);
        if (!ptr)
        {
            Logger::Warn("Attempted to store byte to invalid address: {:08x}", vaddr);
            return;
        }
        *ptr = data;
    }

    void CPU::store_halfword(uint64_t vaddr, uint16_t data)
    {
        TranslatedAddress paddr = translate_vaddr(vaddr);
        uint16_t* ptr = reinterpret_cast<uint16_t*>(cpubus_.redirect_paddress(paddr.paddr));
        if (!ptr)
        {
            Logger::Fatal("Attempted to store halfword to invalid address: {:08x}", vaddr);
        }
        data = hydra::bswap16(data);
        memcpy(ptr, &data, sizeof(uint16_t));
    }

    void CPU::store_word(uint64_t vaddr, uint32_t data)
    {
        TranslatedAddress paddr = translate_vaddr(vaddr);
        uint32_t* ptr = reinterpret_cast<uint32_t*>(cpubus_.redirect_paddress(paddr.paddr));
        bool isviewer = paddr.paddr <= ISVIEWER_AREA_END && paddr.paddr >= ISVIEWER_FLUSH;
        if (!ptr || isviewer)
        {
            write_hwio(paddr.paddr, data);
        }
        else
        {
            data = hydra::bswap32(data);
            memcpy(ptr, &data, sizeof(uint32_t));
        }
    }

    void CPU::store_doubleword(uint64_t vaddr, uint64_t data)
    {
        TranslatedAddress paddr = translate_vaddr(vaddr);
        uint64_t* ptr = reinterpret_cast<uint64_t*>(cpubus_.redirect_paddress(paddr.paddr));
        if (!ptr)
        {
            Logger::Fatal("Attempted to store doubleword to invalid address: {:08x}", vaddr);
        }
        data = hydra::bswap64(data);
        memcpy(ptr, &data, sizeof(uint64_t));
    }

    void CPU::Tick()
    {
        ++cpubus_.time_;
        cpubus_.time_ &= 0x1FFFFFFFF;
        if (cpubus_.time_ == (cp0_regs_[CP0_COMPARE].UD << 1)) [[unlikely]]
        {
            CP0Cause.IP7 = true;
            update_interrupt_check();
        }
        gpr_regs_[0].UD = 0;
        prev_branch_ = was_branch_;
        was_branch_ = false;
        TranslatedAddress paddr = translate_vaddr(pc_);
        uint8_t* ptr = cpubus_.redirect_paddress(paddr.paddr);
        instruction_.full = hydra::bswap32(*reinterpret_cast<uint32_t*>(ptr));
        if (check_interrupts())
        {
            return;
        }
        log_cpu_state<CPU_LOGGING>(true, 30'000'000, 0);
        prev_pc_ = pc_;
        pc_ = next_pc_;
        next_pc_ += 4;
        execute_instruction();
    }

    void CPU::check_vi_interrupt()
    {
        if ((rcp_.vi_.vi_v_current_ & 0x3fe) == rcp_.vi_.vi_v_intr_)
        {
            Logger::Debug("Raising VI interrupt");
            set_interrupt(InterruptType::VI, true);
        }
    }

    bool CPU::check_interrupts()
    {
        if (should_service_interrupt_)
        {
            throw_exception(pc_, ExceptionType::Interrupt);
            return true;
        }
        return false;
    }

    void CPU::conditional_branch(bool condition, uint64_t address)
    {
        was_branch_ = true;
        if (condition)
        {
            branch_to(address);
        }
    }

    void CPU::conditional_branch_likely(bool condition, uint64_t address)
    {
        if (condition)
        {
            branch_to(address);
        }
        else
        {
            pc_ += 4;
            next_pc_ = pc_ + 4;
        }
    }

    void CPU::link_register(uint8_t reg)
    {
        uint64_t sepc = static_cast<int64_t>(static_cast<int32_t>(pc_));
        gpr_regs_[reg].UD = sepc + 4;
    }

    void CPU::branch_to(uint64_t address)
    {
        next_pc_ = address;
        was_branch_ = true;
    }

    void CPU::execute_instruction()
    {
        (instruction_table_[instruction_.IType.op])(this);
    }

    void CPU::execute_cp0_instruction()
    {
        uint32_t func = instruction_.RType.rs;
        if (func & 0b10000)
        {
            // Coprocessor function
            switch (static_cast<CP0Instruction>(instruction_.RType.func))
            {
                case CP0Instruction::ERET:
                {
                    if ((gpr_regs_[CP0_STATUS].UD & 0b10) == 1)
                    {
                        pc_ = cp0_regs_[CP0_ERROREPC].UD;
                        CP0Status.ERL = false;
                    }
                    else
                    {
                        pc_ = cp0_regs_[CP0_EPC].UD;
                        CP0Status.EXL = false;
                    }
                    if (!translate_vaddr(pc_).success)
                    {
                        Logger::Fatal("ERET jumped to invalid address {:016X}", pc_);
                    }
                    next_pc_ = pc_ + 4;
                    llbit_ = 0;
                    update_interrupt_check();
                    break;
                }
                case CP0Instruction::TLBWI:
                {
                    uint8_t index = cp0_regs_[CP0_INDEX].UD & 0b11111;

                    TLBEntry entry;
                    EntryLo el0, el1;
                    EntryHi eh;
                    uint16_t mask = (cp0_regs_[CP0_PAGEMASK].UD >> 13) & 0b101010101010;
                    mask |= mask >> 1;
                    entry.mask = mask;
                    el0.full = cp0_regs_[CP0_ENTRYLO0].UD;
                    el1.full = cp0_regs_[CP0_ENTRYLO1].UD;
                    eh.full = cp0_regs_[CP0_ENTRYHI].UD;
                    entry.G = el0.G && el1.G;
                    entry.entry_even.full = el0.full & 0x3FF'FFFE;
                    entry.entry_odd.full = el1.full & 0x3FF'FFFE;
                    eh.VPN2 &= ~entry.mask;
                    entry.entry_hi.full = eh.full;
                    entry.initialized = true;

                    tlb_[index] = entry;
                    break;
                }
                case CP0Instruction::TLBP:
                {
                    cp0_regs_[CP0_INDEX].UD = 1 << 31;
                    for (int i = 0; i < 32; i++)
                    {
                        const TLBEntry& entry = tlb_[i];
                        if (!entry.initialized)
                        {
                            continue;
                        }
                        EntryHi eh;
                        eh.full = cp0_regs_[CP0_ENTRYHI].UD;

                        if (entry.entry_hi.VPN2 == (eh.VPN2 & ~entry.mask) &&
                            (entry.G || (eh.ASID == entry.entry_hi.ASID)) &&
                            (entry.entry_hi.R == eh.R))
                        {
                            cp0_regs_[CP0_INDEX].UD = i;
                            break;
                        }
                    }
                    break;
                }
                case CP0Instruction::TLBR:
                {
                    uint8_t index = cp0_regs_[CP0_INDEX].UD & 0b11111;

                    TLBEntry entry = tlb_[index];
                    EntryLo el0, el1;
                    el0.full = entry.entry_even.full;
                    el1.full = entry.entry_odd.full;
                    el0.G = entry.G;
                    el1.G = entry.G;
                    cp0_regs_[CP0_ENTRYLO0].UD = el0.full & 0x3FFF'FFFF;
                    cp0_regs_[CP0_ENTRYLO1].UD = el1.full & 0x3FFF'FFFF;
                    cp0_regs_[CP0_ENTRYHI].UD = entry.entry_hi.full;
                    cp0_regs_[CP0_PAGEMASK].UD = entry.mask << 13;

                    break;
                }
                case CP0Instruction::TLBWR:
                {
                    return Logger::Warn("TLBWR is not implemented");
                }
                case CP0Instruction::WAIT:
                {
                    return Logger::Warn("WAIT is not implemented");
                }
                default:
                    Logger::Fatal("Invalid CP0 instruction at {:016X}", pc_);
            }
        }
        else
        {
            switch (func & 0b1111)
            {
                case 0:
                    return MFC0();
                case 1:
                    return DMFC0();
                case 4:
                    return MTC0();
                case 5:
                    return DMTC0();
                default:
                    Logger::Fatal("Invalid CP0 instruction at {:016X}", pc_);
            }
        }
    }

    void CPU::dump_pif_ram()
    {
        std::stringstream ss;
        ss << std::hex << std::setfill('0') << std::setw(2);
        for (int i = 0; i < 64; i++)
        {
            ss << std::setfill('0') << std::setw(2) << static_cast<int>(cpubus_.pif_ram_[i]);
        }
        std::cout << ss.str() << std::endl;
    }

    void CPU::set_cp0_regs_exception(int64_t vaddr)
    {
        cp0_regs_[CP0_BADVADDR].D = vaddr;
        CP0Context.BadVPN2 = (vaddr >> 13) & 0x7FFFF;
        CP0XContext.BadVPN2 = (vaddr >> 13) & 0x7FFFFFF;
        CP0XContext.R = (vaddr >> 62) & 0b11;
        CP0EntryHi.VPN2 = (vaddr >> 13) & 0x7FFFFFF;
        CP0EntryHi.R = (vaddr >> 62) & 0b11;
    }

    bool CPU::is_kernel_mode()
    {
        return (CP0Status.KSU == 0b00) || CP0Status.EXL || CP0Status.ERL;
    }

    void CPU::dump_tlb()
    {
        int i = 0;
        for (const auto& entry : tlb_)
        {
            if (entry.entry_odd.full == 0 && entry.entry_even.full == 0)
            {
                continue;
            }
            Logger::Info("TLB entry {}:\n"
                         "EntryHi: {:016x}\n"
                         "Mask: {:04x}\n"
                         "EntryLoOdd: {:016x}\n"
                         "EntryLoEven: {:016x}\n"
                         "VPN2: {:x}\n",
                         i++, entry.entry_hi.full, entry.mask, entry.entry_odd.full,
                         entry.entry_even.full, entry.entry_hi.VPN2 << 13);
        }
    }

    TranslatedAddress CPU::probe_tlb(uint32_t vaddr)
    {
        for (const TLBEntry& entry : tlb_)
        {
            if (!entry.initialized)
            {
                continue;
            }
            uint32_t vpn_mask = ~((entry.mask << 13) | 0x1FFF);
            uint64_t current_vpn = vaddr & vpn_mask;
            uint64_t have_vpn = (entry.entry_hi.VPN2 << 13) & vpn_mask;
            int current_asid = CP0EntryHi.ASID;
            bool global = entry.G;
            if ((have_vpn == current_vpn) && (global || (entry.entry_hi.ASID == current_asid)))
            {
                uint32_t offset_mask = ((entry.mask << 12) | 0xFFF);
                bool odd = vaddr & (offset_mask + 1);
                EntryLo elo;
                if (odd)
                {
                    if (!entry.entry_odd.V)
                    {
                        return {};
                    }
                    elo.full = entry.entry_odd.full;
                }
                else
                {
                    if (!entry.entry_even.V)
                    {
                        return {};
                    }
                    elo.full = entry.entry_even.full;
                }
                uint32_t paddr = (elo.PFN << 12) | (vaddr & offset_mask);
                return {
                    .paddr = paddr,
                    .cached = elo.C != 2,
                    .success = true,
                };
            }
        }
        Logger::Warn("TLB miss at {:08x}", vaddr);
        return {};
    }

    void CPU::dump_rdram()
    {
        printf("rdram:\n");
        for (int i = 0; i < 0x80'000; i += 4)
        {
            printf("%08x %08x %08x %08x\n", cpubus_.rdram_[i], cpubus_.rdram_[i + 1],
                   cpubus_.rdram_[i + 2], cpubus_.rdram_[i + 3]);
        }
    }

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
        }
        else
        {
            Logger::Warn("Nested exception");
            exit(1);
        }
        CP0Cause.ExCode = static_cast<uint8_t>(type);
        CP0Cause.CE = processor;
        CP0Status.EXL = true;
        update_interrupt_check();
        switch (type)
        {
            case ExceptionType::CoprocessorUnusable:
            {
                Logger::WarnOnce("Coprocessor unusable exception {:08x}", instruction_.full);
                goto handler;
            }
            case ExceptionType::FloatingPoint:
            {
                Logger::Warn("Floating point exception {:08x}", instruction_.full);
                goto handler;
            }
            case ExceptionType::Trap:
            case ExceptionType::Syscall:
            case ExceptionType::Interrupt:
            case ExceptionType::Breakpoint:
            case ExceptionType::TLBMissLoad:
            case ExceptionType::IntegerOverflow:
            case ExceptionType::AddressErrorLoad:
            case ExceptionType::AddressErrorStore:
            case ExceptionType::ReservedInstruction:
            handler:
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

#define rdreg (gpr_regs_[instruction_.RType.rd])
#define rsreg (gpr_regs_[instruction_.RType.rs])
#define rtreg (gpr_regs_[instruction_.RType.rt])
#define saval (instruction_.RType.sa)
#define immval (instruction_.IType.immediate)
#define seimmval (static_cast<int64_t>(static_cast<int16_t>(instruction_.IType.immediate)))
#define fmtval (instruction_.FType.fmt)
#define ftreg (fpr_regs_[instruction_.FType.ft])
#define fsreg (fpr_regs_[instruction_.FType.fs])
#define fdreg (fpr_regs_[instruction_.FType.fd])

    void CPU::LDL()
    {
        int16_t offset = immval;
        uint64_t address = rsreg.D + offset;
        int shift = 8 * ((address ^ 0) & 7);
        uint64_t mask = (uint64_t)0xFFFFFFFFFFFFFFFF << shift;
        uint64_t data = load_doubleword(address & ~7);
        rtreg.UD = (rtreg.UD & ~mask) | (data << shift);
    }

    void CPU::LDR()
    {
        int16_t offset = immval;
        uint64_t address = rsreg.D + offset;
        int shift = 8 * ((address ^ 7) & 7);
        uint64_t mask = (uint64_t)0xFFFFFFFFFFFFFFFF >> shift;
        uint64_t data = load_doubleword(address & ~7);
        rtreg.UD = (rtreg.UD & ~mask) | (data >> shift);
    }

    void CPU::LWL()
    {
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint64_t address = seoffset + rsreg.UD;
        uint32_t shift = 8 * ((address ^ 0) & 3);
        uint32_t mask = 0xFFFFFFFF << shift;
        uint32_t data = load_word(address & ~3);
        rtreg.UD =
            static_cast<int64_t>(static_cast<int32_t>((rtreg.UW._0 & ~mask) | data << shift));
    }

    void CPU::LWR()
    {
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint64_t address = seoffset + rsreg.UD;
        uint32_t shift = 8 * ((address ^ 3) & 3);
        uint32_t mask = 0xFFFFFFFF >> shift;
        uint32_t data = load_word(address & ~3);
        rtreg.UD =
            static_cast<int64_t>(static_cast<int32_t>((rtreg.UW._0 & ~mask) | data >> shift));
    }

    void CPU::SB()
    {
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint64_t address = seoffset + rsreg.UD;
        store_byte(address, rtreg.UB._0);
    }

    void CPU::SWL()
    {
        constexpr static uint32_t mask[4] = {0x00000000, 0xFF000000, 0xFFFF0000, 0xFFFFFF00};
        constexpr static uint32_t shift[4] = {0, 8, 16, 24};
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint64_t address = (static_cast<uint32_t>(seoffset) & ~0b11) + rsreg.UD;
        auto addr_off = immval & 0b11;
        uint32_t word = load_word(address);
        word &= mask[addr_off];
        word |= rtreg.UW._0 >> shift[addr_off];
        store_word(address, word);
    }

    void CPU::SWR()
    {
        constexpr static uint32_t mask[4] = {0x00FFFFFF, 0x0000FFFF, 0x000000FF, 0x00000000};
        constexpr static uint32_t shift[4] = {24, 16, 8, 0};
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint64_t address = (static_cast<uint32_t>(seoffset) & ~0b11) + rsreg.UD;
        auto addr_off = immval & 0b11;
        uint32_t word = load_word(address);
        word &= mask[addr_off];
        word |= rtreg.UW._0 << shift[addr_off];
        store_word(address, word);
    }

    void CPU::SDL()
    {
        constexpr static uint64_t mask[8] = {0,
                                             0xFF00000000000000,
                                             0xFFFF000000000000,
                                             0xFFFFFF0000000000,
                                             0xFFFFFFFF00000000,
                                             0xFFFFFFFFFF000000,
                                             0xFFFFFFFFFFFF0000,
                                             0xFFFFFFFFFFFFFF00};
        constexpr static uint32_t shift[8] = {0, 8, 16, 24, 32, 40, 48, 56};
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint64_t address = (static_cast<uint32_t>(seoffset) & ~0b111) + rsreg.UD;
        auto addr_off = immval & 0b111;
        uint64_t doubleword = load_doubleword(address);
        doubleword &= mask[addr_off];
        doubleword |= rtreg.UD >> shift[addr_off];
        store_doubleword(address, doubleword);
    }

    void CPU::SDR()
    {
        constexpr static uint64_t mask[8] = {
            0x00FFFFFFFFFFFFFF, 0x0000FFFFFFFFFFFF, 0x000000FFFFFFFFFF, 0x00000000FFFFFFFF,
            0x0000000000FFFFFF, 0x000000000000FFFF, 0x00000000000000FF, 0x0000000000000000};
        constexpr static uint32_t shift[8] = {56, 48, 40, 32, 24, 16, 8, 0};
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint64_t address = (static_cast<uint32_t>(seoffset) & ~0b111) + rsreg.UD;
        auto addr_off = immval & 0b111;
        uint64_t doubleword = load_doubleword(address);
        doubleword &= mask[addr_off];
        doubleword |= rtreg.UD << shift[addr_off];
        store_doubleword(address, doubleword);
    }

    void CPU::SD()
    {
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint64_t address = seoffset + rsreg.UD;
        if ((address & 0b111) != 0)
        {
            // If either of the loworder two bits of the address are not zero, an address error
            // exception occurs.
            set_cp0_regs_exception(address);
            throw_exception(prev_pc_, ExceptionType::AddressErrorStore);
            return;
        }
        if (!mode64_ && opmode_ != OperatingMode::Kernel)
        {
            // This operation is defined for the VR4300 operating in 64-bit mode and in 32-bit
            // Kernel mode. Execution of this instruction in 32-bit User or Supervisor mode
            // causes a reserved instruction exception.
            throw_exception(prev_pc_, ExceptionType::ReservedInstruction);
        }
        store_doubleword(address, rtreg.UD);
    }

    void CPU::SW()
    {
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint64_t address = seoffset + rsreg.UD;
        if ((address & 0b11) != 0)
        {
            // If either of the loworder two bits of the address are not zero, an address error
            // exception occurs.
            set_cp0_regs_exception(address);
            throw_exception(prev_pc_, ExceptionType::AddressErrorStore);
            return;
        }
        if ((address >> 31) && static_cast<int64_t>(address) > 0)
        {
            // If bit 31 is set and address is positive, that means it's not sign extended, an
            // address error exception occurs.
            set_cp0_regs_exception(address);
            throw_exception(prev_pc_, ExceptionType::AddressErrorStore);
            return;
        }
        store_word(address, rtreg.UW._0);
    }

    void CPU::SH()
    {
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint64_t address = seoffset + rsreg.UD;
        if ((address & 0b1) != 0)
        {
            // If either of the loworder two bits of the address are not zero, an address error
            // exception occurs.
            set_cp0_regs_exception(address);
            throw_exception(prev_pc_, ExceptionType::AddressErrorStore);
            return;
        }
        store_halfword(address, rtreg.UH._0);
    }

    void CPU::SC()
    {
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint64_t address = seoffset + rsreg.UD;

        if ((address & 0b11) != 0)
        {
            // If either of the loworder two bits of the address are not zero, an address error
            // exception occurs.
            set_cp0_regs_exception(address);
            throw_exception(prev_pc_, ExceptionType::AddressErrorStore);
            return;
        }

        if (llbit_)
        {
            store_word(address, rtreg.UW._0);
            rtreg.UD = 1;
        }
        else
        {
            rtreg.UD = 0;
        }
    }

    void CPU::LBU()
    {
        int16_t offset = immval;
        int32_t seoffset = offset;
        rtreg.UD = load_byte(seoffset + rsreg.UD);
    }

    void CPU::LB()
    {
        int16_t offset = immval;
        int32_t seoffset = offset;
        rtreg.D = static_cast<int8_t>(load_byte(seoffset + rsreg.UD));
    }

    void CPU::LHU()
    {
        int16_t offset = immval;
        int32_t seoffset = offset;
        rtreg.UD = load_halfword(seoffset + rsreg.UD);
    }

    void CPU::LH()
    {
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint64_t address = seoffset + rsreg.UD;
        if ((address & 0b1) != 0)
        {
            // If the least-significant bit of the address is not zero, an address error exception
            // occurs.
            set_cp0_regs_exception(address);
            throw_exception(prev_pc_, ExceptionType::AddressErrorLoad);
            return;
        }
        rtreg.D = static_cast<int16_t>(load_halfword(address));
    }

    void CPU::LWU()
    {
        int16_t offset = immval;
        int32_t seoffset = offset;
        rtreg.UD = load_word(seoffset + rsreg.UD);
    }

    void CPU::LW()
    {
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint64_t address = seoffset + rsreg.UD;
        if ((address & 0b11) != 0)
        {
            // If either of the loworder two bits of the address are not zero, an address error
            // exception occurs.
            set_cp0_regs_exception(address);
            throw_exception(prev_pc_, ExceptionType::AddressErrorLoad);
            return;
        }
        if ((address >> 31) && static_cast<int64_t>(address) > 0)
        {
            // If bit 31 is set and address is positive, that means it's not sign extended, an
            // address error exception occurs.
            set_cp0_regs_exception(address);
            throw_exception(prev_pc_, ExceptionType::AddressErrorLoad);
            return;
        }
        rtreg.D = static_cast<int32_t>(load_word(address));
    }

    void CPU::LD()
    {
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint64_t address = seoffset + rsreg.UD;
        if ((address & 0b111) != 0)
        {
            // If either of the loworder two bits of the address are not zero, an address error
            // exception occurs.
            set_cp0_regs_exception(address);
            throw_exception(prev_pc_, ExceptionType::AddressErrorLoad);
            return;
        }
        if (!mode64_ && opmode_ != OperatingMode::Kernel)
        {
            // This operation is defined for the VR4300 operating in 64-bit mode and in 32-bit
            // Kernel mode. Execution of this instruction in 32-bit User or Supervisor mode
            // causes a reserved instruction exception.
            throw_exception(prev_pc_, ExceptionType::ReservedInstruction);
            return;
        }
        rtreg.UD = load_doubleword(address);
    }

    void CPU::LL()
    {
        int16_t offset = immval;
        int32_t seoffset = offset;
        uint64_t address = seoffset + rsreg.UD;
        if ((address & 0b11) != 0)
        {
            // If either of the loworder two bits of the address are not zero, an address error
            // exception occurs.
            set_cp0_regs_exception(address);
            throw_exception(prev_pc_, ExceptionType::AddressErrorLoad);
            return;
        }
        rtreg.D = static_cast<int32_t>(load_word(address));
        llbit_ = true;
        lladdr_ = translate_vaddr(address).paddr;
    }

    void CPU::BGTZL()
    {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch_likely(rsreg.D > 0, pc_ + seoffset);
    }

    void CPU::BLEZ()
    {
        int32_t seoffset = static_cast<int16_t>(immval << 2);
        conditional_branch(rsreg.D <= 0, pc_ + seoffset);
    }

    void CPU::BEQ()
    {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch(rsreg.UD == rtreg.UD, pc_ + seoffset);
    }

    void CPU::BEQL()
    {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch_likely(rsreg.UD == rtreg.UD, pc_ + seoffset);
    }

    void CPU::BNE()
    {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch(rsreg.UD != rtreg.UD, pc_ + seoffset);
    }

    void CPU::BNEL()
    {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch_likely(rsreg.UD != rtreg.UD, pc_ + seoffset);
    }

    void CPU::BLEZL()
    {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch_likely(rsreg.D <= 0, pc_ + seoffset);
    }

    void CPU::BGTZ()
    {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch(rsreg.D > 0, pc_ + seoffset);
    }

    void CPU::JAL()
    {
        link_register(31);
        J();
    }

    void CPU::J()
    {
        auto jump_addr = instruction_.JType.target;
        // combine first 3 bits of pc and jump_addr shifted left by 2
        branch_to(((pc_ - 4) & 0xF000'0000) | (jump_addr << 2));
    }

    void CPU::s_JALR()
    {
        link_register(instruction_.RType.rd);
        // Register numbers rs and rd should not be equal, because such an instruction does
        // not have the same effect when re-executed. If they are equal, the contents of rs
        // are destroyed by storing link address. However, if an attempt is made to execute
        // this instruction, an exception will not occur, and the result of executing such an
        // instruction is undefined.
        if (rdreg.UD != rsreg.UD)
        {
            // throw_exception(prev_pc_, ExceptionType::AddressErrorLoad);
        }
        s_JR();
    }

    void CPU::s_JR()
    {
        auto jump_addr = rsreg.UD;
        if ((jump_addr & 0b11) != 0)
        {
            // Since instructions must be word-aligned, a Jump Register instruction must
            // specify a target register (rs) which contains an address whose low-order two bits
            // are zero. If these low-order two bits are not zero, an address exception will occur
            // when the jump target instruction is fetched.
            set_cp0_regs_exception(jump_addr);
            throw_exception(jump_addr, ExceptionType::AddressErrorLoad);
            return;
        }
        branch_to(jump_addr);
    }

    void CPU::r_BLTZ()
    {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch(rsreg.D < 0, pc_ + seoffset);
    }

    void CPU::r_BGEZ()
    {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch(rsreg.W._0 >= 0, pc_ + seoffset);
    }

    void CPU::r_BLTZL()
    {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch_likely(rsreg.D < 0, pc_ + seoffset);
    }

    void CPU::r_BGEZL()
    {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch_likely(rsreg.W._0 >= 0, pc_ + seoffset);
    }

    void CPU::r_BLTZAL()
    {
        Logger::Warn("r_BLTZAL not implemented");
    }

    void CPU::r_BGEZAL()
    {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch(rsreg.D >= 0, pc_ + seoffset);
        link_register(31);
    }

    void CPU::r_BLTZALL()
    {
        Logger::Warn("r_BLTZALL not implemented");
    }

    void CPU::r_BGEZALL()
    {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        link_register(31);
        conditional_branch_likely(rsreg.D >= 0, pc_ + seoffset);
    }

    void CPU::s_SLL()
    {
        rdreg.D = static_cast<int32_t>(rtreg.UW._0 << saval);
    }

    void CPU::s_SRL()
    {
        rdreg.D = static_cast<int32_t>(rtreg.UW._0 >> saval);
    }

    void CPU::s_SRA()
    {
        rdreg.D = static_cast<int32_t>(rtreg.D >> saval);
    }

    void CPU::s_SLLV()
    {
        rdreg.D = static_cast<int32_t>(rtreg.UW._0 << (rsreg.UD & 0b11111));
    }

    void CPU::s_SRLV()
    {
        rdreg.D = static_cast<int32_t>(rtreg.UW._0 >> (rsreg.UD & 0b11111));
    }

    void CPU::s_SRAV()
    {
        rdreg.D = static_cast<int32_t>(rtreg.D >> (rsreg.UD & 0b11111));
    }

    void CPU::s_DSRL()
    {
        rdreg.UD = rtreg.UD >> saval;
    }

    void CPU::s_DSRLV()
    {
        rdreg.UD = rtreg.UD >> (rsreg.UD & 0b111111);
    }

    void CPU::s_DSLL()
    {
        rdreg.UD = rtreg.UD << saval;
    }

    void CPU::s_DSLLV()
    {
        rdreg.UD = rtreg.UD << (rsreg.UD & 0b111111);
    }

    void CPU::s_DSLL32()
    {
        rdreg.UD = rtreg.UD << (saval + 32);
    }

    void CPU::s_DSRA()
    {
        rdreg.D = rtreg.D >> saval;
    }

    void CPU::s_DSRA32()
    {
        rdreg.D = rtreg.D >> (saval + 32);
    }

    void CPU::s_DSRAV()
    {
        rdreg.D = rtreg.D >> (rsreg.UD & 0b111111);
    }

    void CPU::s_DSRL32()
    {
        rdreg.UD = rtreg.UD >> (saval + 32);
    }

    void CPU::s_ADD()
    {
        int32_t result = 0;
        bool overflow = hydra::add_overflow(rtreg.W._0, rsreg.W._0, result);
        if (overflow)
        {
            return throw_exception(prev_pc_, ExceptionType::IntegerOverflow);
        }
        rdreg.UD = static_cast<int64_t>(result);
    }

    void CPU::s_ADDU()
    {
        rdreg.D = static_cast<int32_t>(rsreg.UW._0 + rtreg.UW._0);
    }

    void CPU::s_DADD()
    {
        int64_t result = 0;
        bool overflow = hydra::add_overflow(rtreg.D, rsreg.D, result);
        if (overflow)
        {
            return throw_exception(prev_pc_, ExceptionType::IntegerOverflow);
        }
        rdreg.D = result;
    }

    void CPU::s_DADDU()
    {
        rdreg.UD = rsreg.UD + rtreg.UD;
    }

    void CPU::s_SUB()
    {
        int32_t result = 0;
        bool overflow = hydra::sub_overflow(rsreg.W._0, rtreg.W._0, result);
        if (overflow)
        {
            return throw_exception(prev_pc_, ExceptionType::IntegerOverflow);
        }
        rdreg.D = result;
    }

    void CPU::s_SUBU()
    {
        rdreg.D = static_cast<int32_t>(rsreg.UW._0 - rtreg.UW._0);
    }

    void CPU::s_DSUB()
    {
        int64_t result = 0;
        bool overflow = hydra::sub_overflow(rsreg.D, rtreg.D, result);
        if (overflow)
        {
            return throw_exception(prev_pc_, ExceptionType::IntegerOverflow);
        }
        rdreg.D = result;
    }

    void CPU::s_DSUBU()
    {
        rdreg.UD = rsreg.UD - rtreg.UD;
    }

    void CPU::s_MULT()
    {
        uint64_t res = static_cast<int64_t>(rsreg.W._0) * rtreg.W._0;
        lo_ = static_cast<int64_t>(static_cast<int32_t>(res & 0xFFFF'FFFF));
        hi_ = static_cast<int64_t>(static_cast<int32_t>(res >> 32));
    }

    void CPU::s_MULTU()
    {
        uint64_t res = static_cast<uint64_t>(rsreg.UW._0) * rtreg.UW._0;
        lo_ = static_cast<int64_t>(static_cast<int32_t>(res & 0xFFFF'FFFF));
        hi_ = static_cast<int64_t>(static_cast<int32_t>(res >> 32));
    }

    void CPU::s_DMULT()
    {
        auto [high, low] = hydra::mul64(rsreg.D, rtreg.D);
        hi_ = high;
        lo_ = low;
    }

    void CPU::s_DMULTU()
    {
        auto [high, low] = hydra::mul64(rsreg.UD, rtreg.UD);
        hi_ = high;
        lo_ = low;
    }

    void CPU::s_DIV()
    {
        if (rtreg.W._0 == 0) [[unlikely]]
        {
            lo_ = rsreg.W._0 < 0 ? 1 : -1;
            hi_ = static_cast<int64_t>(rsreg.W._0);
            return;
        }
        // TODO: replace with uint64_t division
        if (rsreg.W._0 == std::numeric_limits<decltype(rsreg.W._0)>::min() && rtreg.W._0 == -1)
            [[unlikely]]
        {
            lo_ = static_cast<int64_t>(rsreg.W._0);
            hi_ = 0;
            return;
        }
        lo_ = rsreg.W._0 / rtreg.W._0;
        hi_ = rsreg.W._0 % rtreg.W._0;
    }

    void CPU::s_DIVU()
    {
        if (rtreg.UW._0 == 0) [[unlikely]]
        {
            lo_ = -1;
            hi_ = rsreg.UD;
            return;
        }
        lo_ = rsreg.UW._0 / rtreg.UW._0;
        hi_ = rsreg.UW._0 % rtreg.UW._0;
    }

    void CPU::s_DDIV()
    {
        if (rtreg.D == 0) [[unlikely]]
        {
            bool sign = rsreg.UD >> 63;
            lo_ = sign ? 1 : -1;
            hi_ = rsreg.UD;
            return;
        }
        if (rsreg.D == std::numeric_limits<int64_t>::min() && rtreg.D == -1)
        {
            lo_ = rsreg.D;
            hi_ = 0;
            return;
        }
        lo_ = rsreg.D / rtreg.D;
        hi_ = rsreg.D % rtreg.D;
    }

    void CPU::s_DDIVU()
    {
        if (rtreg.UD == 0) [[unlikely]]
        {
            lo_ = -1;
            hi_ = rsreg.UD;
            return;
        }
        lo_ = static_cast<int64_t>(rsreg.UD / rtreg.UD);
        hi_ = static_cast<int64_t>(rsreg.UD % rtreg.UD);
    }

    void CPU::s_AND()
    {
        rdreg.UD = rsreg.UD & rtreg.UD;
    }

    void CPU::s_OR()
    {
        rdreg.UD = rsreg.UD | rtreg.UD;
    }

    void CPU::s_XOR()
    {
        rdreg.UD = rsreg.UD ^ rtreg.UD;
    }

    void CPU::s_NOR()
    {
        rdreg.UD = ~(rsreg.UD | rtreg.UD);
    }

    void CPU::s_TGE()
    {
        if (rsreg.D >= rtreg.D)
        {
            throw_exception(prev_pc_, ExceptionType::Trap);
        }
    }

    void CPU::s_TGEU()
    {
        if (rsreg.UD >= rtreg.UD)
        {
            throw_exception(prev_pc_, ExceptionType::Trap);
        }
    }

    void CPU::s_TLT()
    {
        if (rsreg.D < rtreg.D)
        {
            throw_exception(prev_pc_, ExceptionType::Trap);
        }
    }

    void CPU::s_TLTU()
    {
        if (rsreg.UD < rtreg.UD)
        {
            throw_exception(prev_pc_, ExceptionType::Trap);
        }
    }

    void CPU::s_TEQ()
    {
        if (rsreg.UD == rtreg.UD)
        {
            throw_exception(prev_pc_, ExceptionType::Trap);
        }
    }

    void CPU::s_TNE()
    {
        if (rsreg.UD != rtreg.UD)
        {
            throw_exception(prev_pc_, ExceptionType::Trap);
        }
    }

    void CPU::r_TGEI()
    {
        if (rsreg.D >= seimmval)
        {
            throw_exception(prev_pc_, ExceptionType::Trap);
        }
    }

    void CPU::r_TGEIU()
    {
        if (rsreg.UD >= static_cast<uint64_t>(seimmval))
        {
            throw_exception(prev_pc_, ExceptionType::Trap);
        }
    }

    void CPU::r_TLTI()
    {
        if (rsreg.D < seimmval)
        {
            throw_exception(prev_pc_, ExceptionType::Trap);
        }
    }

    void CPU::r_TLTIU()
    {
        if (rsreg.UD < static_cast<uint64_t>(seimmval))
        {
            throw_exception(prev_pc_, ExceptionType::Trap);
        }
    }

    void CPU::r_TEQI()
    {
        if (rsreg.D == seimmval)
        {
            throw_exception(prev_pc_, ExceptionType::Trap);
        }
    }

    void CPU::r_TNEI()
    {
        if (rsreg.D != seimmval)
        {
            throw_exception(prev_pc_, ExceptionType::Trap);
        }
    }

    void CPU::s_SLT()
    {
        rdreg.UD = rsreg.D < rtreg.D;
    }

    void CPU::s_SLTU()
    {
        rdreg.UD = rsreg.UD < rtreg.UD;
    }

    void CPU::s_SYSCALL()
    {
        throw_exception(prev_pc_, ExceptionType::Syscall);
    }

    void CPU::s_BREAK()
    {
        throw_exception(prev_pc_, ExceptionType::Breakpoint);
    }

    void CPU::s_SYNC() {}

    void CPU::s_MFHI()
    {
        rdreg.UD = hi_;
    }

    void CPU::s_MTHI()
    {
        hi_ = rsreg.UD;
    }

    void CPU::s_MFLO()
    {
        rdreg.UD = lo_;
    }

    void CPU::s_MTLO()
    {
        lo_ = rsreg.UD;
    }

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
            case 0x9:
            case 0xA:
            case 0xB:
            case 0xC:
            case 0xD:
            case 0xE:
            case 0xF:
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
        bool overflow = hydra::add_overflow(rsreg.W._0, seimm, result);
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
        bool overflow = hydra::add_overflow(rsreg.D, seimm, result);
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
                CP0CauseType newcause;
                newcause.full = cp0_regs_[reg].UD;
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
            case 21:
            case 22:
            case 23:
            case 24:
            case 25:
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
                update_interrupt_check();
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
                update_interrupt_check();
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
                update_interrupt_check();
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

    enum
    {
        FMT_S = 16,
        FMT_D = 17,
        FMT_W = 20,
        FMT_L = 21
    };

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

    template <class T>
    void CPU::check_fpu_result(T& arg)
    {
        switch (std::fpclassify(arg))
        {
            case FP_NAN:
            {
                arg = get_nan<T>();
                break;
            }
            case FP_SUBNORMAL:
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
                fcr31_.cause_inexact = 1;
                if (!fcr31_.enable_inexact)
                {
                    fcr31_.flag_inexact = 1;
                }
                switch (fcr31_.rounding_mode)
                {
                    case 0:
                    case 1:
                    {
                        arg = std::copysign(0, arg);
                        break;
                    }
                    case 2:
                    {
                        if (std::signbit(arg))
                        {
                            arg = -static_cast<T>(0);
                        }
                        else
                        {
                            arg = std::numeric_limits<T>::min();
                        }
                        break;
                    }
                    case 3:
                    {
                        if (std::signbit(arg))
                        {
                            arg = -std::numeric_limits<T>::min();
                        }
                        else
                        {
                            arg = 0;
                        }
                        break;
                    }
                }
                break;
            }
        }
    }

    template <class T>
    bool CPU::check_nan(T arg)
    {
        if constexpr (std::is_same_v<T, float>)
        {
            return (hydra::bit_cast<uint32_t>(arg) & 0x7FC00000) == 0x7FC00000;
        }
        else
        {
            return (hydra::bit_cast<uint64_t>(arg) & 0x7FF8000000000000) == 0x7FF8000000000000;
        }
    }

    template <class T>
    T CPU::get_nan()
    {
        if constexpr (std::is_same_v<T, float>)
        {
            return hydra::bit_cast<float>(0x7FBF'FFFF);
        }
        else
        {
            return hydra::bit_cast<double>(0x7FF7'FFFF'FFFF'FFFFu);
        }
    }

    template <class T>
    void CPU::check_fpu_arg(T arg)
    {
        switch (std::fpclassify(arg))
        {
            case FP_NAN:
            {
                if (check_nan<T>(arg))
                {
                    fcr31_.cause_invalidop = 1;
                    if (!fcr31_.enable_invalidop)
                    {
                        fcr31_.flag_invalidop = 1;
                    }
                }
                else
                {
                    fcr31_.unimplemented = 1;
                }
                break;
            }
            case FP_SUBNORMAL:
            {
                fcr31_.unimplemented = 1;
                break;
            }
            default:
            {
                break;
            }
        }
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
        int round_mode = std::fegetround();
        switch (fcr31_.rounding_mode)
        {
            case 0:
            {
                std::fesetround(FE_TONEAREST);
                break;
            }
            case 1:
            {
                std::fesetround(FE_TOWARDZERO);
                break;
            }
            case 2:
            {
                std::fesetround(FE_UPWARD);
                break;
            }
            case 3:
            {
                std::fesetround(FE_DOWNWARD);
                break;
            }
        }
        std::feclearexcept(FE_ALL_EXCEPT);
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
        std::fesetround(round_mode);
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

#undef rdreg
#undef rsreg
#undef rtreg
#undef saval
#undef immval
#undef seimmval
#undef fmtval
#undef ftreg
#undef fsreg
#undef fdreg
} // namespace hydra::N64