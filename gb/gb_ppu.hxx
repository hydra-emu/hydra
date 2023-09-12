#pragma once

#include <array>
#include <gb/gb_addresses.hxx>
#include <gb/gb_bus.hxx>
#include <mutex>
#include <queue>

namespace hydra::Gameboy
{
    constexpr int FRAME_CYCLES = 70224;

    class PPU
    {
    public:
        bool ReadyToDraw = false;
        bool SpriteDebugColor = false;
        bool DrawBackground = true;
        bool DrawWindow = true;
        bool DrawSprites = true;
        bool UseCGB = false;
        PPU(Bus& bus);
        void Update(uint8_t cycles);
        void Reset();
        uint8_t* GetScreenData();
        void FillTileset(float* pixels, size_t x_off = 0, size_t y_off = 0, uint16_t addr = 0x8000);

    private:
        Bus& bus_;
        std::vector<uint8_t> screen_color_data_{};
        std::vector<uint8_t> screen_color_data_second_{};
        // PPU memory mapped registers
        uint8_t &LCDC, &STAT, &LYC, &LY, &IF, &SCY, &SCX, &WY, &WX;
        std::vector<uint8_t> cur_scanline_sprites_;
        uint8_t window_internal_temp_ = 0;
        uint8_t window_internal_ = 0;
        int clock_ = 0;
        int clock_target_ = 0;
        int set_mode(int mode);
        int get_mode();
        int update_lyc();
        bool is_sprite_eligible(uint8_t sprite_y);
        void draw_scanline();
        PaletteColors& get_cur_bg_pal(uint8_t attributes);
        PaletteColors& get_cur_obj_pal(uint8_t attributes);
        inline void render_tiles();
        inline void render_sprites();
    };
} // namespace hydra::Gameboy
