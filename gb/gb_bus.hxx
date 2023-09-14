#pragma once

#include <array>
#include <cstdint>
#include <deque>
#include <fstream>
#include <gb/gb_addresses.hxx>
#include <gb/gb_apu.hxx>
#include <gb/gb_apu_ch.hxx>
#include <gb/gb_cartridge.hxx>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace hydra
{
    class HydraCore_Gameboy;
}

namespace hydra::Gameboy
{
    struct Change
    {
        int type = 0;
        std::optional<std::array<uint16_t, 4>> new_bg_pal = std::nullopt;
        int new_bg_pal_index = 0;
        std::optional<bool> new_bg_en = std::nullopt;
    };

    using PaletteColors = std::array<uint16_t, 4>;
    class PPU;

    class Bus
    {
    private:
        using RamBank = std::array<uint8_t, 0x2000>;
        using RomBank = std::array<uint8_t, 0x4000>;

    public:
        bool BiosEnabled = false;
        Bus(ChannelArrayPtr channel_array_ptr);
        ~Bus();
        std::string GetVramDump(); // TODO: remove this function, switch to QA struct for test
        uint8_t Read(uint16_t address);
        uint16_t ReadL(uint16_t address);
        uint8_t& GetReference(uint16_t address);
        void ClearNR52Bit(uint8_t bit);
        void Write(uint16_t address, uint8_t data);
        void WriteL(uint16_t address, uint16_t data);
        void TransferDMA(uint8_t clk);
        void TransferHDMA();
        void Reset();
        void SoftReset();
        std::vector<RamBank>& GetRamBanks();
        Cartridge& GetCartridge();
        bool LoadCartridge(const std::string& filename);
        bool LoadCartridge(uint8_t* data);
        std::array<std::array<uint8_t, 3>, 4> Palette;
        std::unordered_map<uint8_t, Change> ScanlineChanges;
        std::array<PaletteColors, 8> BGPalettes{};
        std::array<PaletteColors, 8> OBJPalettes{};
        bool SoundEnabled = false;
        bool DIVReset = false;
        bool TMAChanged = false;
        bool TIMAChanged = false;
        bool WriteToVram = false;
        bool OAMAccessible = true;
        bool UseCGB = false;
        uint8_t DirectionKeys = 0b1110'1111;
        uint8_t ActionKeys = 0b1101'1111;
        uint8_t CurScanlineX = 0;
        uint8_t selected_ram_bank_ = 0;
        uint8_t selected_rom_bank_ = 1;
        uint8_t selected_rom_bank_high_ = 0;

    private:
        bool ram_enabled_ = false;
        bool rtc_enabled_ = false;
        bool banking_mode_ = false;
        bool action_key_mode_ = false;
        bool dma_transfer_ = false;
        bool dma_setup_ = false;
        bool dma_fresh_bug_ = false;
        uint16_t hdma_source_ = 0;
        uint16_t hdma_dest_ = 0;
        uint16_t hdma_index_ = 0;
        uint16_t hdma_size_ = 0;
        bool hdma_transfer_ = false;
        uint8_t hdma_remaining_ = 0;
        bool use_gdma_ = false;
        bool bg_palette_auto_increment_ = false;
        uint8_t bg_palette_index_ = 0;
        bool obj_palette_auto_increment_ = false;
        uint8_t obj_palette_index_ = 0;
        bool vram_sel_bank_ = 0;
        uint8_t wram_sel_bank_ = 1;
        uint8_t rom_banks_size_ = 2;
        std::string curr_save_file_;
        size_t dma_index_ = 0;
        uint16_t dma_offset_ = 0;
        uint16_t dma_new_offset_ = 0;
        uint8_t unused_mem_area_ = 0;
        std::vector<RamBank> ram_banks_;
        std::vector<RomBank> rom_banks_;
        Cartridge cartridge_;
        std::array<uint8_t, 0x100> hram_{};
        std::array<uint8_t, 0x2000> eram_default_{};
        std::array<std::array<uint8_t, 0x1000>, 8> wram_banks_{};
        std::array<std::array<uint8_t, 0x2000>, 2> vram_banks_{};
        std::array<uint8_t, 0xA0> oam_{};
        std::array<uint8_t, 0x40> bg_cram_{};
        std::array<uint8_t, 0x40> obj_cram_{};
        std::array<uint8_t*, 0x100> fast_map_{};
        std::array<uint8_t, 0x100> dmg_bios_{};
        std::array<uint8_t, 0x900> cgb_bios_{};
        bool dmg_bios_loaded_ = false;
        bool cgb_bios_loaded_ = false;
        ChannelArrayPtr channel_array_ptr_;
        uint8_t& redirect_address(uint16_t address);
        uint8_t& fast_redirect_address(uint16_t address);
        void fill_fast_map();
        inline void refill_fast_map_rom();
        inline void refill_fast_map_vram();
        inline void refill_fast_map_wram();

        void handle_mbc(uint16_t address, uint8_t data);
        void battery_save();
        // Take channel input with 1-based index to match the register names (eg. NR14)
        void handle_nrx4(int channel_no, uint8_t& data);
        void handle_nrx2(int channel_no, uint8_t& data);
        void handle_nrx1(int channel_no, uint8_t& data);
        void disable_dac(int channel_no);

        friend class PPU;
        friend class hydra::HydraCore_Gameboy;
    };
} // namespace hydra::Gameboy
