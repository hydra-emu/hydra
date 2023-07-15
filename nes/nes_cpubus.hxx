#pragma once

#include "nes_apu.hxx"
#include "nes_joypad.hxx"
#include "nes_ppu.hxx"
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace hydra::NES
{
    enum class Mapper {
        NROM = 0,

        MMC3 = 4,
    };
    class CPU;

    struct Header
    {
        char id[4];
        uint8_t prg_lsb;
        uint8_t chr_lsb;
        uint8_t flags_6;
        uint8_t flags_7;
        uint8_t mapper_msb;
        uint8_t prg_chr_msb;
        uint8_t prgram_eeprom_sz;
        uint8_t chrram_sz;
        uint8_t cpu_ppu_timing;
        uint8_t byte_13;
        uint8_t misc_roms;
        uint8_t default_expansion;
    };

    class CPUBus
    {
    public:
        CPUBus(PPU& ppu, APU& apu);
        bool LoadCartridge(std::string path);
        void Reset();

    private:
        uint8_t read(uint16_t addr);
        inline uint8_t redirect_address_r(uint16_t addr);
        inline void redirect_address_w(uint16_t addr, uint8_t data);
        void write(uint16_t addr, uint8_t data);
        void refill_prg_map();

        std::array<uint8_t, 512> trainer_;
        std::vector<uint8_t> prg_rom_;
        uint8_t last_read_;
        uint16_t mapper_ = 0;
        Header header_;
        std::array<uint8_t*, 0x100> fast_map_;
        std::array<uint8_t, 0x800> ram_;
        PPU& ppu_;
        APU& apu_;
        Joypad joypad1_, joypad2_;
        friend class CPU;
    };
} // namespace hydra::NES
