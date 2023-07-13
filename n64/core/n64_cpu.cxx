#include "n64_cpu.hxx"
#include <cstring>
#include <cassert>
#include <iostream>
#include <bitset>
#include <limits>
#include <sstream>
#include <cmath>
#include <bit>
#include <iomanip>
#include <boost/stacktrace/stacktrace.hpp>
#include "utils.hxx"
#include "n64_sm64_helper.hxx"

namespace hydra::N64 {

    template<>
    void CPU::log_cpu_state<false>(bool, uint64_t, uint64_t) {}

    template<>
    void CPU::log_cpu_state<true>(bool use_crc, uint64_t instructions, uint64_t start) {
        static uint64_t count = 0;
        count++;
        if (count >= start + instructions) {
            exit(1);
        }
        if (count < start) {
            return;
        }
        // Get crc32 of gpr and fpr regs
        if (use_crc) {
            uint32_t gprcrc = 0xFFFF'FFFF;
            uint32_t fprcrc = 0xFFFF'FFFF;
            uint32_t pifcrc = 0xFFFF'FFFF;
            for (int i = 0; i < 32; i++) {
                gprcrc = _mm_crc32_u64(gprcrc, gpr_regs_[i].UD);
                fprcrc = _mm_crc32_u64(fprcrc, fpr_regs_[i].UD);
            }
            for (int i = 0; i < 64; i++) {
                // pifcrc = _mm_crc32_u8(pifcrc, cpubus_.pif_ram_[i]);
            }
            gprcrc ^= 0xFFFF'FFFF;
            fprcrc ^= 0xFFFF'FFFF;
            pifcrc ^= 0xFFFF'FFFF;
            printf("%08x %08x %08x %08x", pc_, instruction_.full, gprcrc, fprcrc);
        } else {
            printf("%08x %08x ", pc_, instruction_.full);
            for (int i = 1; i < 32; i++) {
                printf("%016x ", gpr_regs_[i].UD);
            }
        }
        printf("\n");
    }

    void CPU::write_hwio(uint32_t addr, uint32_t data) {
        switch (addr) {
            case RSP_DMA_SPADDR: return rcp_.rsp_.write_hwio(RSPHWIO::Cache, data);
            case RSP_DMA_RAMADDR: return rcp_.rsp_.write_hwio(RSPHWIO::DramAddr, data);
            case RSP_DMA_RDLEN: return rcp_.rsp_.write_hwio(RSPHWIO::RdLen, data);
            case RSP_DMA_WRLEN: return rcp_.rsp_.write_hwio(RSPHWIO::WrLen, data);
            case RSP_STATUS: return rcp_.rsp_.write_hwio(RSPHWIO::Status, data);
            case RSP_SEMAPHORE: return rcp_.rsp_.write_hwio(RSPHWIO::Semaphore, data);
            case RSP_PC: {
                if (!rcp_.rsp_.status_.halt) {
                    Logger::Warn("RSP PC write while not halted");
                }
                rcp_.rsp_.pc_ = data & 0xffc;
                rcp_.rsp_.next_pc_ = rcp_.rsp_.pc_ + 4;
                break;
            }
            case PI_STATUS: {
                if (data & 0b10) {
                    cpubus_.mi_interrupt_.PI = false;
                }
                break;
            }
            case PI_DRAM_ADDR: {
                cpubus_.pi_dram_addr_ = data;
                break;
            }
            case PI_CART_ADDR: {
                cpubus_.pi_cart_addr_ = data;
                break;
            }
            case PI_RD_LEN: {
                // std::memcpy(&cpubus_.rdram_[__builtin_bswap32(cpubus_.pi_cart_addr_)], cpubus_.redirect_paddress(__builtin_bswap32(cpubus_.pi_dram_addr_)), data + 1);
                Logger::Warn("PI_RD_LEN write");
                cpubus_.mi_interrupt_.PI = true;
                break;
            }
            case PI_WR_LEN: {
                auto cart_addr = cpubus_.pi_cart_addr_ & 0xFFFFFFFE;
                auto dram_addr = cpubus_.pi_dram_addr_ & 0x007FFFFE;
                uint64_t length = data + 1;
                if (cart_addr >= 0x8000000 && cart_addr < 0x10000000) {
                    Logger::Warn("DMA to SRAM is unimplemented!");
                    cpubus_.dma_busy_ = false;
                    cpubus_.mi_interrupt_.PI = true;
                    return;
                }
                std::memcpy(&cpubus_.rdram_[dram_addr], cpubus_.redirect_paddress(cart_addr), length);
                cpubus_.dma_busy_ = true;
                uint8_t domain = 0;
                if ((cart_addr >= 0x0800'0000 && cart_addr < 0x1000'0000) || (cart_addr >= 0x0500'0000 && cart_addr < 0x0600'0000)) {
                    domain = 2;
                } else if ((cart_addr >= 0x0600'0000 && cart_addr < 0x0800'0000) || (cart_addr >= 0x1000'0000 && cart_addr < 0x1FC0'0000)) {
                    domain = 1;
                }
                auto cycles = timing_pi_access(domain, length);
                cpubus_.dma_busy_ = false;
                cpubus_.mi_interrupt_.PI = true;
                Logger::Debug("Raising PI interrupt");
                break;
            }
            case PI_BSD_DOM1_PWD: {
                cpubus_.pi_bsd_dom1_pwd_ = data & 0xFF;
                break;
            }
            case PI_BSD_DOM2_PWD: {
                cpubus_.pi_bsd_dom2_lat_ = data & 0xFF;
                break;
            }
            case PI_BSD_DOM1_PGS: {
                cpubus_.pi_bsd_dom1_pgs_ = data & 0xFF;
                break;
            }
            case PI_BSD_DOM2_PGS: {
                cpubus_.pi_bsd_dom2_pgs_ = data & 0xFF;
                break;
            }
            case PI_BSD_DOM1_LAT: {
                cpubus_.pi_bsd_dom1_lat_ = data & 0xFF;
                break;
            }
            case PI_BSD_DOM2_LAT: {
                cpubus_.pi_bsd_dom2_lat_ = data & 0xFF;
                break;
            }
            case PI_BSD_DOM1_RLS: {
                cpubus_.pi_bsd_dom1_rls_ = data & 0xFF;
                break;
            }
            case PI_BSD_DOM2_RLS: {
                cpubus_.pi_bsd_dom2_rls_ = data & 0xFF;
                break;
            }
            case VI_CTRL: {
                auto format = data & 0b11;
                rcp_.SetPixelMode(format);
                if ((data >> 6) & 0b1) {
                    Logger::Warn("Interlacing enabled");
                }
                break;
            }
            case VI_ORIGIN: {
                data &= 0x00FFFFFF;
                if (data > cpubus_.rdram_.size()) {
                    Logger::Fatal("VI_ORIGIN out of bounds: {:#x}", data);
                }
                rcp_.vi_.memory_ptr_ = &cpubus_.rdram_[data];
                break;
            }
            case VI_WIDTH: {
                rcp_.vi_.vi_width_ = data;
                break;
            }
            case VI_V_CURRENT: {
                cpubus_.mi_interrupt_.VI = false;
                break;
            }
            case VI_H_SYNC:
            case VI_H_SYNC_LEAP:
            case VI_V_BURST:
            case VI_TEST_ADDR:
            case VI_STAGED_DATA:
            case VI_BURST: {
                break;
            }
            case VI_V_INTR: {
                rcp_.vi_.vi_v_intr_ = data & 0x3ff;
                break;
            }
            case VI_V_SYNC: {
                rcp_.vi_.num_halflines_ = data >> 1;
                rcp_.vi_.cycles_per_halfline_ = (93750000 / 60) / rcp_.vi_.num_halflines_;
                break;
            }
            case VI_H_VIDEO: {
                rcp_.SetHVideo(data);
                break;
            }
            case VI_V_VIDEO: {
                rcp_.SetVVideo(data);
                break;
            }
            case VI_X_SCALE: {
                rcp_.SetXScale(data);
                break;
            }
            case VI_Y_SCALE: {
                rcp_.SetYScale(data);
                break;
            }
            case AI_AREA_START ... AI_AREA_END: {
                return rcp_.ai_.WriteWord(addr, data);
            }
            case MI_MODE: {
                // TODO: properly implement
                cpubus_.mi_mode_ = data;

                if ((data >> 11) & 0b1) {
                    cpubus_.mi_interrupt_.DP = false;
                }
                break;
            }
            case MI_MASK: {
                for (int j = 2, i = 0; i < 6; i++) {
                    if (data & j) {
                        cpubus_.mi_mask_ |= 1 << i;
                    }
                    j <<= 2;
                }
                for (int j = 1, i = 0; i < 6; i++) {
                    if (data & j) {
                        cpubus_.mi_mask_ &= ~(1 << i);
                    }
                    j <<= 2;
                }
                break;
            }
            case SI_DRAM_ADDR: {
                cpubus_.si_dram_addr_ = data;
                break;
            }
            case SI_PIF_AD_WR64B: {
                std::memcpy(cpubus_.pif_ram_.data(), &cpubus_.rdram_[cpubus_.si_dram_addr_ & 0xff'ffff], 64);
                pif_command();
                cpubus_.mi_interrupt_.SI = true;
                Logger::Debug("Raising SI interrupt");
                break;
            }
            case SI_PIF_AD_RD64B: {
                pif_command();
                std::memcpy(&cpubus_.rdram_[cpubus_.si_dram_addr_ & 0xff'ffff], cpubus_.pif_ram_.data(), 64);
                cpubus_.mi_interrupt_.SI = true;
                Logger::Debug("Raising SI interrupt");
                break;
            }
            case SI_STATUS: {
                cpubus_.mi_interrupt_.SI = false;
                break;
            }
            case RI_MODE:
            case RI_CONFIG:
            case RI_CURRENT_LOAD: 
            case RI_SELECT:
            case RI_REFRESH: 
            case RI_LATENCY: {
                Logger::Warn("Write to RI register {:x} with data {:x}", addr, data);
                break;
            }
            case RDP_AREA_START ... RDP_AREA_END: {
                rcp_.rdp_.WriteWord(addr, data);
                break;
            }
            case PIF_START ... PIF_END: {
                uint32_t* ptr = reinterpret_cast<uint32_t*>(&cpubus_.pif_ram_[addr - PIF_START]);
                *ptr = __builtin_bswap32(data);
                pif_command();
                break;
            }
            case PIF_COMMAND: {
                cpubus_.pif_ram_[63] = data;
                pif_command();
                break;
            }
            case ISVIEWER_START ... ISVIEWER_END: {
                data = __builtin_bswap32(data);
                for (int i = 0; i < 4; i++) {
                    cpubus_.isviewer_buffer_[addr - ISVIEWER_START + i] = data >> (i * 8);
                }
                break;
            }
            case ISVIEWER_FLUSH: {
                std::stringstream ss;
                for (int i = 0; i < data; i++) {
                    ss << cpubus_.isviewer_buffer_[i];
                }
                std::cout << ss.str();
                break;
            }
            case RDRAM_REGISTERS_START ... RDRAM_REGISTERS_END: {
                Logger::Warn("Write to RDRAM register {:x} with data {:x}", addr, data);
                break;
            }
            case RDRAM_BROADCAST_START ... RDRAM_BROADCAST_END: {
                Logger::Warn("Write to RDRAM broadcast register {:x} with data {:x}", addr, data);
                break;
            }
            default: {
                Logger::Warn("Unhandled write_hwio to address: {:08x} {:08x}", addr, data);
                break;
            }
        }
    }

    uint32_t CPU::read_hwio(uint32_t paddr) {
        #define redir_case(addr, data) case addr: return data;
        switch (paddr) {
            // MIPS Interface
            redir_case(MI_MODE, cpubus_.mi_mode_);
            case MI_VERSION: {
                return 0x02020102;
            }
            redir_case(MI_INTERRUPT, cpubus_.mi_interrupt_.full);
            redir_case(MI_MASK, cpubus_.mi_mask_);

            // Video Interface
            redir_case(VI_CTRL, rcp_.vi_.vi_ctrl_);
            redir_case(VI_ORIGIN, rcp_.vi_.vi_origin_);
            redir_case(VI_WIDTH, rcp_.vi_.vi_width_);
            redir_case(VI_V_INTR, rcp_.vi_.vi_v_intr_);
            redir_case(VI_V_CURRENT, rcp_.vi_.vi_v_current_);
            redir_case(VI_BURST, rcp_.vi_.vi_burst_);
            redir_case(VI_V_SYNC, rcp_.vi_.vi_v_sync_);
            redir_case(VI_H_SYNC, rcp_.vi_.vi_h_sync_);
            redir_case(VI_H_SYNC_LEAP, rcp_.vi_.vi_h_sync_leap_);
            case VI_H_VIDEO: {
                return rcp_.vi_.vi_h_end_ | (rcp_.vi_.vi_h_start_ << 16);
            }
            case VI_V_VIDEO: {
                return rcp_.vi_.vi_v_end_ | (rcp_.vi_.vi_v_start_ << 16);
            }
            redir_case(VI_V_BURST, rcp_.vi_.vi_v_burst_);
            redir_case(VI_X_SCALE, rcp_.vi_.vi_x_scale_)
            redir_case(VI_Y_SCALE, rcp_.vi_.vi_y_scale_);
            redir_case(VI_TEST_ADDR, rcp_.vi_.vi_test_addr_);
            redir_case(VI_STAGED_DATA, rcp_.vi_.vi_staged_data_);

            // Audio Interface
            case AI_AREA_START ... AI_AREA_END: {
                return rcp_.ai_.ReadWord(paddr);
            }

            // Peripheral Interface
            redir_case(PI_DRAM_ADDR, cpubus_.pi_dram_addr_);
            redir_case(PI_CART_ADDR, cpubus_.pi_cart_addr_);
            redir_case(PI_RD_LEN, cpubus_.pi_rd_len_);
            redir_case(PI_WR_LEN, cpubus_.pi_wr_len_);
            case PI_STATUS: {
                return cpubus_.dma_busy_ | (cpubus_.io_busy_ << 1) | (cpubus_.dma_error_ << 2) | (cpubus_.mi_interrupt_.PI << 3);
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
            case RSP_DMA_SPADDR: return rcp_.rsp_.read_hwio(RSPHWIO::Cache);
            case RSP_DMA_RAMADDR: return rcp_.rsp_.read_hwio(RSPHWIO::DramAddr);
            case RSP_DMA_RDLEN: return rcp_.rsp_.read_hwio(RSPHWIO::RdLen);
            case RSP_DMA_WRLEN: return rcp_.rsp_.read_hwio(RSPHWIO::WrLen);
            case RSP_STATUS: return rcp_.rsp_.read_hwio(RSPHWIO::Status);
            case RSP_DMA_FULL: return rcp_.rsp_.read_hwio(RSPHWIO::Full);
            case RSP_DMA_BUSY: return rcp_.rsp_.read_hwio(RSPHWIO::Busy);
            case RSP_SEMAPHORE: return rcp_.rsp_.read_hwio(RSPHWIO::Semaphore);
            case PIF_START ... PIF_END: {
                return __builtin_bswap32(*reinterpret_cast<uint32_t*>(&cpubus_.pif_ram_[paddr - PIF_START]));
            }
            redir_case(PIF_COMMAND, cpubus_.pif_ram_[63]);
            case RSP_PC: {
                if (!rcp_.rsp_.status_.halt) {
                    Logger::Warn("Reading from RSP_PC while not halted");
                }
                return rcp_.rsp_.pc_;
            }
            case RDP_AREA_START ... RDP_AREA_END: {
                return rcp_.rdp_.ReadWord(paddr);
            }
            case ISVIEWER_FLUSH:
            case ISVIEWER_START ... ISVIEWER_END: {
                Logger::Fatal("Reading from ISViewer");
            }
            case RDRAM_REGISTERS_START ... RDRAM_REGISTERS_END: {
                Logger::Warn("Reading from RDRAM registers");
                return 0;
            }
            case 0x05000000 ... 0x05FFFFFF: {
                Logger::Warn("Accessing N64DD");
                return 0;
            }
            case 0x08000000 ... 0x0FFFFFFF: {
                Logger::Warn("Accessing SRAM");
                return 0;
            }
            default: {
                break;
            }
        }
        #undef redir_case
        Logger::Warn("Unhandled read_hwio from address {:08x} PC: {:08x}", paddr, pc_);
        return 0;
    }

    enum class JoybusCommand : uint8_t {
        RequestInfo = 0,
        ControllerState = 1,
        ReadMempack = 2,
        WriteMempack = 3,
        ReadEEPROM = 4,
        WriteEEPROM = 5,
        Reset = 255,
    };

    void CPU::pif_command() {
        using namespace hydra::N64;
        auto command_byte = cpubus_.pif_ram_[63];
        if (command_byte & 0x1) {
            pif_channel_ = 0;
            int i = 0;
            while (i < 63) {
                int initial_i = i;
                int8_t tx = cpubus_.pif_ram_[i++];
                if (tx > 0) {
                    uint8_t* rx_ptr = &cpubus_.pif_ram_[i++];
                    if (*rx_ptr == 0xFE) {
                        break;
                    }
                    uint8_t rx = *rx_ptr & 0x3F;
                    std::vector<uint8_t> data;
                    // get tx next bytes and send to pif chip
                    data.resize(tx);
                    for (int j = 0; j < tx; j++) {
                        data[j] = cpubus_.pif_ram_[i++];
                    }
                    // get response bytes
                    std::vector<uint8_t> response;
                    response.resize(rx);
                    if (joybus_command(data, response)) {
                        // Device not found
                        rx_ptr[0] |= 0x80;
                    }
                    for (int j = 0; j < response.size(); j++) {
                        cpubus_.pif_ram_[i++] = response[j];
                    }
                    pif_channel_++;
                } else if (tx == 0) {
                    pif_channel_++;
                    continue;
                } else if (tx == 0xFE) {
                    break;
                }
            }
        }
        if (command_byte & 0x8) {
            command_byte &= ~8;
        }
        if (command_byte & 0x20) {
            // cpubus_.pif_ram_[0x32] = 0;
            // cpubus_.pif_ram_[0x33] = 0;
            // cpubus_.pif_ram_[0x34] = 0;
            // cpubus_.pif_ram_[0x35] = 0;
            // cpubus_.pif_ram_[0x36] = 0;
            // cpubus_.pif_ram_[0x37] = 0;
        }
        if (command_byte & 0x30) {
            command_byte = 0x80;
        }
        cpubus_.pif_ram_[63] = command_byte;
    }

    bool CPU::joybus_command(const std::vector<uint8_t>& command, std::vector<uint8_t>& result) {
        if (result.size() == 0) {
            Logger::Fatal("Joybus command with no result");
        }
        JoybusCommand command_type = static_cast<JoybusCommand>(command[0]);
        switch (command_type) {
            case JoybusCommand::Reset:
            case JoybusCommand::RequestInfo: {
                if (result.size() != 3) {
                    dump_pif_ram();
                    Logger::Fatal("Joybus RequestInfo command with result size {}", result.size());
                }
                switch (pif_channel_) {
                    case 0: {
                        result[0] = 0x05;
                        result[1] = 0x00;
                        result[2] = 0x01;
                        break;
                    }
                    case 4: {
                        result[0] = 0x00;
                        result[1] = 0x80;
                        result[2] = 0x00;
                        break;
                    }
                }
                break;
            }
            case JoybusCommand::ControllerState: {
                if (result.size() != 4) {
                    Logger::Fatal("Joybus ControllerState command with result size {}", result.size());
                }
                if (pif_channel_ != 0) {
                    return true;
                }
                result[0] = key_state_[Keys::A] << 7 |
                    key_state_[Keys::B] << 6 |
                    key_state_[Keys::Z] << 5 |
                    key_state_[Keys::Start] << 4;
                    // TODO: DPAD
                    // |
                    // key_state_[Keys::Up] << 3 |
                    // key_state_[Keys::Down] << 2 |
                    // key_state_[Keys::Left] << 1 |
                    // key_state_[Keys::Right];
                result[1] = 0 | 0 |
                    key_state_[Keys::L] << 5 |
                    key_state_[Keys::R] << 4 |
                    key_state_[Keys::CUp] << 3 |
                    key_state_[Keys::CDown] << 2 |
                    key_state_[Keys::CLeft] << 1 |
                    key_state_[Keys::CRight];
                int8_t x = 0, y = 0;
                if (key_state_[Keys::Up]) {
                    y = 127;
                } else if (key_state_[Keys::Down]) {
                    y = -127;
                }
                if (key_state_[Keys::Left]) {
                    x = -127;
                } else if (key_state_[Keys::Right]) {
                    x = 127;
                }
                result[2] = x;
                result[3] = y;
                break;
            }
            case JoybusCommand::WriteMempack: {
                if (result.size() != 1) {
                    Logger::Fatal("Joybus WriteMempack command with result size {}", result.size());
                }
                result[0] = 0x80;
                break;
            }
            default: {
                Logger::Warn("Unhandled joybus command type {}", static_cast<uint8_t>(command_type));
            }
        }
        return false;
    }

    CPU::CPU(CPUBus& cpubus, RCP& rcp, bool& should_draw) :
        gpr_regs_{},
        fpr_regs_{},
        instr_cache_(KB(16)),
        data_cache_(KB(8)),
        cpubus_(cpubus),
        rcp_(rcp),
        should_draw_(should_draw)
    {
        rcp_.ai_.InstallBuses(&cpubus_.rdram_[0]);
        rcp_.rsp_.InstallBuses(&cpubus_.rdram_[0], &rcp_.rdp_);
        rcp_.rdp_.InstallBuses(&cpubus_.rdram_[0], &rcp_.rsp_.mem_[0]);
        rcp_.ai_.SetMIPtr(&cpubus_.mi_interrupt_);
        rcp_.rsp_.SetMIPtr(&cpubus_.mi_interrupt_);
        rcp_.rdp_.SetMIPtr(&cpubus_.mi_interrupt_);
    }

    void CPU::Reset() {
        pc_ = 0xFFFF'FFFF'BFC0'0000;
        next_pc_ = pc_ + 4;
        for (auto& reg : gpr_regs_) {
            reg.UD = 0;
        }
        for (auto& reg : fpr_regs_) {
            reg.UD = 0;
        }
        for (auto& reg : cp0_regs_) {
            reg.UD = 0;
        }
        cpubus_.Reset();
        rcp_.Reset();
        CP0Status.full = 0x3400'0000;
        CP0Cause.full = 0xB000'007C;
        cp0_regs_[CP0_EPC].UD = 0xFFFF'FFFF'FFFF'FFFFu;
        cp0_regs_[CP0_ERROREPC].UD = 0xFFFF'FFFF'FFFF'FFFFu;
        cp0_regs_[CP0_CONFIG].UD = 0x7006'E463;
        cp0_regs_[CP0_PRID].UD = 0x0000'0B22;
        CP0Context.full = 0;
        CP0XContext.full = 0;
        CP0EntryHi.full = 0;
        for (auto& entry : tlb_) {
            TLBEntry newentry {};
            newentry.initialized = false;
            std::swap(entry, newentry);
        }
        store_word(0x8000'0318, 0x800000); // TODO: probably done by pif somewhere if RI_SELECT is emulated or something
    }

    // Shamelessly stolen from dillon
    // Thanks m64p
    uint32_t CPU::timing_pi_access(uint8_t domain, uint32_t length) {
        uint32_t cycles = 0;
        uint32_t latency = 0;
        uint32_t pulse_width = 0;
        uint32_t release = 0;
        uint32_t page_size = 0;
        uint32_t pages = 0;
        switch (domain) {
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

    TranslatedAddress CPU::translate_vaddr(uint32_t addr) {
        if (is_kernel_mode()) [[likely]] {
            return translate_vaddr_kernel(addr);
        } else {
            Logger::Fatal("Non kernel mode :(");
        }
        return {};
    }

    TranslatedAddress CPU::translate_vaddr_kernel(uint32_t addr) {
        if (addr >= 0x80000000 && addr <= 0xBFFFFFFF) [[likely]] {
            return { addr & 0x1FFFFFFF, true, true };
        } else if (addr >= 0 && addr <= 0x7FFFFFFF) {
            // User segment
            TranslatedAddress paddr = probe_tlb(addr);
            if (!paddr.success) {
                throw_exception(prev_pc_, ExceptionType::TLBMissLoad);
                set_cp0_regs_exception(addr);
            }
            return paddr;
        } else if (addr >= 0xC0000000 && addr <= 0xDFFFFFFF) {
            // Supervisor segment
            Logger::Warn("Accessing supervisor segment {:08x}", addr);
        } else {
            // Kernel segment TLB
            Logger::Warn("Accessing kernel segment {:08x}", addr);
        }
        return {};
    }

    uint8_t CPU::load_byte(uint64_t vaddr) {
        TranslatedAddress paddr = translate_vaddr(vaddr);
        uint8_t* ptr = cpubus_.redirect_paddress(paddr.paddr);
        if (!ptr) {
            Logger::Fatal("Attempted to load byte from invalid address: {:08x}", vaddr);
        }
        return *ptr;
    }

    uint16_t CPU::load_halfword(uint64_t vaddr) {
        TranslatedAddress paddr = translate_vaddr(vaddr);
        uint16_t* ptr = reinterpret_cast<uint16_t*>(cpubus_.redirect_paddress(paddr.paddr));
        if (!ptr) {
            Logger::Fatal("Attempted to load halfword from invalid address: {:08x}", vaddr);
        }
        return __builtin_bswap16(*ptr);
    }

    uint32_t CPU::load_word(uint64_t vaddr) {
        TranslatedAddress paddr = translate_vaddr(vaddr);
        uint32_t* ptr = reinterpret_cast<uint32_t*>(cpubus_.redirect_paddress(paddr.paddr));
        if (!ptr) {
            return read_hwio(paddr.paddr);
        } else {
            return __builtin_bswap32(*ptr);
        }
    }

    uint64_t CPU::load_doubleword(uint64_t vaddr) {
        TranslatedAddress paddr = translate_vaddr(vaddr);
        uint64_t* ptr = reinterpret_cast<uint64_t*>(cpubus_.redirect_paddress(paddr.paddr));
        if (!ptr) {
            Logger::Fatal("Attempted to load doubleword from invalid address: {:08x}", vaddr);
        }
        return __builtin_bswap64(*ptr);
    }

    void CPU::store_byte(uint64_t vaddr, uint8_t data) {
        TranslatedAddress paddr = translate_vaddr(vaddr);
        uint8_t* ptr = cpubus_.redirect_paddress(paddr.paddr);
        if (!ptr) {
            Logger::Warn("Attempted to store byte to invalid address: {:08x}", vaddr);
            return;
        }
        *ptr = data;
    }

    void CPU::store_halfword(uint64_t vaddr, uint16_t data) {
        TranslatedAddress paddr = translate_vaddr(vaddr);
        uint16_t* ptr = reinterpret_cast<uint16_t*>(cpubus_.redirect_paddress(paddr.paddr));
        if (!ptr) {
            Logger::Fatal("Attempted to store halfword to invalid address: {:08x}", vaddr);
        }
        *ptr = __builtin_bswap16(data);
    }

    void CPU::store_word(uint64_t vaddr, uint32_t data) {
        TranslatedAddress paddr = translate_vaddr(vaddr);
        uint32_t* ptr = reinterpret_cast<uint32_t*>(cpubus_.redirect_paddress(paddr.paddr));
        bool isviewer = paddr.paddr <= ISVIEWER_END && paddr.paddr >= ISVIEWER_FLUSH;
        if (!ptr || isviewer) {
            write_hwio(paddr.paddr, data);
        } else {
            *ptr = __builtin_bswap32(data);
        }
    }

    void CPU::store_doubleword(uint64_t vaddr, uint64_t data) {
        TranslatedAddress paddr = translate_vaddr(vaddr);
        uint64_t* ptr = reinterpret_cast<uint64_t*>(cpubus_.redirect_paddress(paddr.paddr));
        if (!ptr) {
            Logger::Fatal("Attempted to store doubleword to invalid address: {:08x}", vaddr);
        }
        *ptr = __builtin_bswap64(data);
    }

    void CPU::Tick() {
        if (rcp_.ai_.IsHungry()) {
            ++cpubus_.time_;
            cpubus_.time_ &= 0x1FFFFFFFF;
            if (cpubus_.time_ == (cp0_regs_[CP0_COMPARE].UD << 1)) [[unlikely]] {
                CP0Cause.IP7 = true;
            }
            while (scheduler_.size() && cpubus_.time_ >= scheduler_.top().time) [[unlikely]]
                handle_event();
            gpr_regs_[0].UD = 0;
            prev_branch_ = was_branch_;
            was_branch_ = false;
            instruction_.full = load_word(pc_);
            std::optional<std::string> name_func = std::nullopt;// SM64Helper::Probe(pc_);
            if (check_interrupts()) {
                return;
            }
            log_cpu_state<CPU_LOGGING>(true, 50'000'000, 200'000'000);
            prev_pc_ = pc_;
            pc_ = next_pc_;
            next_pc_ += 4;
            if (name_func) {
                std::cout << "Running " << *name_func << std::endl;
            }
            execute_instruction();
        }
    }

    void CPU::check_vi_interrupt() {
        if ((rcp_.vi_.vi_v_current_ & 0x3fe) == rcp_.vi_.vi_v_intr_) {
            Logger::Debug("Raising VI interrupt");
            cpubus_.mi_interrupt_.VI = true;
        }
    }

    bool CPU::check_interrupts() {
        bool mi_interrupt = cpubus_.mi_interrupt_.full & cpubus_.mi_mask_;
        CP0Cause.IP2 = mi_interrupt;
        bool interrupts_pending = cp0_regs_[CP0_CAUSE].UB._1 & CP0Status.IM;
        bool interrupts_enabled = CP0Status.IE;
        bool currently_handling_exception = CP0Status.EXL;
        bool currently_handling_error = CP0Status.ERL;
        bool should_service_interrupt = interrupts_pending
                                && interrupts_enabled
                                && !currently_handling_exception
                                && !currently_handling_error;
        if (should_service_interrupt) {
            throw_exception(pc_, ExceptionType::Interrupt);
            return true;
        }
        return false;
    }

    void CPU::conditional_branch(bool condition, uint64_t address) {
        was_branch_ = true;
        if (condition) {
            branch_to(address);
        }
    }

    void CPU::conditional_branch_likely(bool condition, uint64_t address) {
        if (condition) {
            branch_to(address);
        } else {
            pc_ += 4;
            next_pc_ = pc_ + 4;
        }
    }

    void CPU::link_register(uint8_t reg) {
        uint64_t sepc = static_cast<int64_t>(static_cast<int32_t>(pc_));
        gpr_regs_[reg].UD = sepc + 4;
    }

    void CPU::branch_to(uint64_t address) {
        next_pc_ = address;
        was_branch_ = true;
    }

    void CPU::execute_instruction() {
        (instruction_table_[instruction_.IType.op])(this);
    }

    void CPU::execute_cp0_instruction() {
        uint32_t func = instruction_.RType.rs;
        if (func & 0b10000) {
            // Coprocessor function
            switch (static_cast<CP0Instruction>(instruction_.RType.func)) {
                case CP0Instruction::ERET: {
                    if ((gpr_regs_[CP0_STATUS].UD & 0b10) == 1) {
                        pc_ = cp0_regs_[CP0_ERROREPC].UD;
                        CP0Status.ERL = false;
                    } else {
                        pc_ = cp0_regs_[CP0_EPC].UD;
                        CP0Status.EXL = false;
                    }
                    if (!translate_vaddr(pc_).success) {
                        Logger::Fatal("ERET jumped to invalid address {:016X}", pc_);
                    }
                    next_pc_ = pc_ + 4;
                    llbit_ = 0;
                    break;
                }
                case CP0Instruction::TLBWI: {
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
                case CP0Instruction::TLBP: {
                    cp0_regs_[CP0_INDEX].UD = 1 << 31;
                    for (int i = 0; i < 32; i++) {
                        const TLBEntry& entry = tlb_[i];
                        if (!entry.initialized)
                            continue;
                        EntryHi eh;
                        eh.full = cp0_regs_[CP0_ENTRYHI].UD;

                        if (entry.entry_hi.VPN2 == (eh.VPN2 & ~entry.mask) && (entry.G || (eh.ASID == entry.entry_hi.ASID)) && (entry.entry_hi.R == eh.R)) {
                            cp0_regs_[CP0_INDEX].UD = i;
                            break;
                        }
                    }
                }
                case CP0Instruction::TLBR: {
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
                case CP0Instruction::TLBWR: {
                    return Logger::Warn("TLBWR is not implemented");
                }
                case CP0Instruction::WAIT: {
                    return Logger::Warn("WAIT is not implemented");
                }
                default: Logger::Fatal("Invalid CP0 instruction at {:016X}", pc_);
            }
        } else {
            switch (func & 0b1111) {
                case 0: return MFC0();
                case 1: return DMFC0();
                case 4: return MTC0();
                case 5: return DMTC0();
                default: Logger::Fatal("Invalid CP0 instruction at {:016X}", pc_);
            }
        }
    }

    void CPU::dump_pif_ram() {
        std::stringstream ss;
        ss << std::hex << std::setfill('0') << std::setw(2);
        for (int i = 0; i < 64; i++) {
            ss << std::setfill('0') << std::setw(2) << static_cast<int>(cpubus_.pif_ram_[i]);
        }
        std::cout << ss.str() << std::endl;
    }

    void CPU::set_cp0_regs_exception(int64_t vaddr) {
        cp0_regs_[CP0_BADVADDR].D = vaddr;
        CP0Context.BadVPN2 = (vaddr >> 13) & 0x7FFFF;
        CP0XContext.BadVPN2 = (vaddr >> 13) & 0x7FFFFFF;
        CP0XContext.R = (vaddr >> 62) & 0b11;
        CP0EntryHi.VPN2 = (vaddr >> 13) & 0x7FFFFFF;
        CP0EntryHi.R = (vaddr >> 62) & 0b11;
    }

    bool CPU::is_kernel_mode() {
        return (CP0Status.KSU == 0b00) || CP0Status.EXL || CP0Status.ERL;
    }

    void CPU::dump_tlb() {
        int i = 0;
        for (const auto& entry : tlb_) {
            if (entry.entry_odd.full == 0 && entry.entry_even.full == 0) {
                continue;
            }
            Logger::Info(
                "TLB entry {}:\n"
                "EntryHi: {:016x}\n"
                "Mask: {:04x}\n"
                "EntryLoOdd: {:016x}\n"
                "EntryLoEven: {:016x}\n"
                "VPN2: {:x}\n",
                i++,
                entry.entry_hi.full,
                entry.mask,
                entry.entry_odd.full,
                entry.entry_even.full,
                entry.entry_hi.VPN2 << 13
            );
        }
    }

    TranslatedAddress CPU::probe_tlb(uint32_t vaddr) {
        for (const TLBEntry& entry : tlb_) {
            if (!entry.initialized)
                continue;
            uint32_t vpn_mask = ~((entry.mask << 13) | 0x1FFF);
            uint64_t current_vpn = vaddr & vpn_mask;
            uint64_t have_vpn = (entry.entry_hi.VPN2 << 13) & vpn_mask;
            int current_asid = CP0EntryHi.ASID;
            bool global = entry.G;
            if ((have_vpn == current_vpn) && (global || (entry.entry_hi.ASID == current_asid))) {
                uint32_t offset_mask = ((entry.mask << 12) | 0xFFF);
                bool odd = vaddr & (offset_mask + 1);
                EntryLo elo;
                if (odd) {
                    if (!entry.entry_odd.V) {
                        return {};
                    }
                    elo.full = entry.entry_odd.full;
                } else {
                    if (!entry.entry_even.V) {
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

    void CPU::dump_rdram() {
        printf("rdram:\n");
        for (int i = 0; i < 0x80'000; i += 4) {
            printf("%08x %08x %08x %08x\n", cpubus_.rdram_[i], cpubus_.rdram_[i + 1], cpubus_.rdram_[i + 2], cpubus_.rdram_[i + 3]);
        }
    }

}