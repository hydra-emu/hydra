#include <crc32.hxx>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <log.hxx>
#include <n64/core/n64_addresses.hxx>
#include <n64/core/n64_cpu.hxx>
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
            crc = crc32_u8(crc, cart_rom_[i + 0x40]);
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

        // Map cartridge rom
        for (int i = ADDR_TO_PAGE(0x1000'0000); i <= ADDR_TO_PAGE(0x1FB0'0000); i++)
        {
            page_table_[i] = &cart_rom_[PAGE_SIZE * (i - ADDR_TO_PAGE(0x1000'0000))];
        }
#undef ADDR_TO_PAGE
    }
} // namespace hydra::N64