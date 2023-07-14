#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <mutex>

namespace hydra::NES
{
    class CPUBus;

    struct Sprite
    {
        uint8_t y = 0xFF;
        uint8_t tile_index = 0xFF;
        uint8_t attributes = 0xFF;
        uint8_t x = 0xFF;
    };

    class PPU
    {
      public:
        PPU();
        void Tick();
        void Reset();
        uint8_t* GetScreenData();
        void SetNMI(std::function<void()> func);

      private:
        uint8_t ppu_ctrl_ = 0, ppu_mask_ = 0, ppu_status_ = 0, oam_addr_ = 0, oam_data_ = 0,
                ppu_scroll_ = 0, ppu_data_ = 0, oam_dma_ = 0;
        uint16_t vram_addr_ = 0;
        int scanline_ = 0;
        int scanline_cycle_ = 0;
        int pixel_cycle_ = 0;
        uint8_t nt_latch_ = 0;
        uint8_t at_latch_ = 0;
        uint16_t pt_low_latch_ = 0;
        uint16_t pt_high_latch_ = 0;
        uint16_t piso_bg_low_ = 0;
        uint16_t piso_bg_high_ = 0;
        uint8_t piso_at_ = 0;
        uint16_t nt_addr_ = 0;
        uint16_t cur_y_ = 0;
        uint16_t cur_x_ = 0;
        uint16_t fetch_x_ = 0;
        uint16_t fetch_y_ = 0;
        bool vram_incr_vertical_ = false;
        uint16_t sprite_pattern_address_ = 0;
        uint16_t background_pattern_address_ = 0;
        bool sprite_size_ = false;
        bool ppu_master_ = false;
        bool nmi_output_ = false;
        std::array<uint8_t, 0x800> vram_{};
        std::array<uint8_t, 256 * 240 * 4> screen_color_data_;
        std::array<uint8_t, 256 * 240 * 4> screen_color_data_second_;
        std::array<Sprite, 64> oam_{};
        std::array<Sprite, 8> secondary_oam_{};
        // Holds the pattern table data for up to 8 sprites, to be rendered on the current scanline.
        // Unused sprites are loaded with an all-transparent set of values
        std::array<std::pair<uint8_t, uint8_t>, 8> sprite_shift_registers_{};
        // Holds the attribute bytes for up to 8 sprites
        std::array<uint8_t, 8> attribute_latches_{};
        // Holds the X positions for up to 8 sprites on the current scanline
        std::array<uint8_t, 8> sprite_counters_{};
        std::array<bool, 8> sprite_active_{};
        uint8_t scanline_sprite_count_ = 0;
        std::array<std::array<uint8_t, 3>, 0x40> master_palette_;
        std::array<uint8_t, 3> universal_bg_;
        bool draw_tile_grid_ = false;
        bool draw_metatile_grid_ = false;
        bool draw_attribute_grid_ = false;
        using Palettes = std::array<std::array<std::array<uint8_t, 3>, 4>, 4>;
        Palettes background_palettes_{};
        Palettes sprite_palettes_{};
        __always_inline void handle_normal_scanline();
        __always_inline void handle_empty_scanline();
        __always_inline void handle_prerender_scanline();
        __always_inline uint8_t fetch_nt();
        __always_inline uint8_t fetch_at();
        __always_inline uint8_t fetch_pt_low();
        __always_inline uint8_t fetch_pt_high();
        __always_inline void fetch_sprite_data();
        __always_inline void fetch_secondary_oam();
        __always_inline void execute_pipeline();
        __always_inline void draw_pixel();
        __always_inline uint8_t read(uint16_t addr);
        __always_inline void write(uint16_t addr, uint8_t data);
        void dma(std::array<uint8_t, 256> data);
        void invalidate(uint8_t addr, uint8_t data);
        std::vector<uint8_t> chr_rom_;
        std::function<void()> fire_nmi;
        uint64_t master_clock_dbg_ = 0;
        friend class CPUBus;
    };
} // namespace hydra::NES