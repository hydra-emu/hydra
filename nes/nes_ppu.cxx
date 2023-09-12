#include "nes_ppu.hxx"
#include <iostream>
#include <log.hxx>
#include <nes/NTSC_CRT/crt_core.h>

enum ScanlineState
{
    NT_BYTE_LOW = 0,
    NT_BYTE_HIGH = 1,
    AT_BYTE_LOW = 2,
    AT_BYTE_HIGH = 3,
    LOW_BG_BYTE_LOW = 4,
    LOW_BG_BYTE_HIGH = 5,
    HIGH_BG_BYTE_LOW = 6,
    HIGH_BG_BYTE_HIGH = 7,
};

unsigned char reverse(unsigned char b)
{
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

static struct CRT crt;
static struct NTSC_SETTINGS ntsc;
static int color = 1;
// static int noise = 10;
static int field = 0;
static int raw = 0;
static int hue = 0;

namespace hydra::NES
{
    PPU::PPU()
        : master_palette_{{
              {84, 84, 84},    {0, 30, 116},    {8, 16, 144},    {48, 0, 136},    {68, 0, 100},
              {92, 0, 48},     {84, 4, 0},      {60, 24, 0},     {32, 42, 0},     {8, 58, 0},
              {0, 64, 0},      {0, 60, 0},      {0, 50, 60},     {0, 0, 0},       {0, 0, 0},
              {0, 0, 0},       {152, 150, 152}, {8, 76, 196},    {48, 50, 236},   {92, 30, 228},
              {136, 20, 176},  {160, 20, 100},  {152, 34, 32},   {120, 60, 0},    {84, 90, 0},
              {40, 114, 0},    {8, 124, 0},     {0, 118, 40},    {0, 102, 120},   {0, 0, 0},
              {0, 0, 0},       {0, 0, 0},       {236, 238, 236}, {76, 154, 236},  {120, 124, 236},
              {176, 98, 236},  {228, 84, 236},  {236, 88, 180},  {236, 106, 100}, {212, 136, 32},
              {160, 170, 0},   {116, 196, 0},   {76, 208, 32},   {56, 204, 108},  {56, 180, 204},
              {60, 60, 60},    {0, 0, 0},       {0, 0, 0},       {236, 238, 236}, {168, 204, 236},
              {188, 188, 236}, {212, 178, 236}, {236, 174, 236}, {236, 174, 212}, {236, 180, 176},
              {228, 196, 144}, {204, 210, 120}, {180, 222, 120}, {168, 226, 144}, {152, 226, 180},
              {160, 214, 228}, {160, 162, 160}, {0, 0, 0},       {0, 0, 0},
          }}
    {
        crt_init(&crt, 256, 240, CRT_PIX_FORMAT_RGBA, screen_color_data_second_.data());
        crt.blend = 1;
        crt.scanlines = 1;
    }

    void PPU::SetNMI(std::function<void(void)> func)
    {
        fire_nmi = std::move(func);
    }

    void PPU::Tick()
    {
        if (scanline_ <= 239)
        {
            handle_normal_scanline();
            scanline_cycle_++;
        }
        else if (scanline_ == 240)
        {
            handle_empty_scanline();
            scanline_cycle_++;
        }
        else if (scanline_ <= 260)
        {
            if (scanline_ == 241 && scanline_cycle_ == 9)
            {
                ppu_status_ |= 0x80;
                ntsc.data = screen_color_data_second_.data();
                ntsc.w = 256;
                ntsc.h = 240;
                ntsc.format = CRT_PIX_FORMAT_RGBA;
                ntsc.as_color = color;
                ntsc.field = field & 1;
                ntsc.raw = raw;
                ntsc.hue = hue;
                ntsc.yoffset = 0;
                ntsc.xoffset = 0;
                if (ntsc.field == 0)
                {
                    ntsc.frame ^= 1;
                }
                // crt_modulate(&crt, &ntsc);
                // crt_demodulate(&crt, noise);
                field ^= 1;
                std::swap(screen_color_data_, screen_color_data_second_);
            }
            if (nmi_output_)
            {
                nmi_output_ = false;
                fire_nmi();
            }
            handle_empty_scanline();
            scanline_cycle_++;
        }
        else if (scanline_ == 261)
        {
            handle_prerender_scanline();
            scanline_cycle_++;
        }
        master_clock_dbg_++;
    }

    void PPU::handle_normal_scanline()
    {
        if (scanline_cycle_ == 0)
        {
            cur_x_ = 0;
        }
        else if (scanline_cycle_ <= 256)
        {
            fetch_x_ = 2 + (scanline_cycle_ - 1) / 8;
            fetch_y_ = cur_y_ / 8;
            draw_pixel();
            execute_pipeline();
            pixel_cycle_++;
            if (pixel_cycle_ == 8)
            {
                pixel_cycle_ = 0;
            }
        }
        else if (scanline_cycle_ <= 320)
        {
            oam_addr_ = 0;
            if (scanline_cycle_ == 257)
            {
                piso_bg_high_ = 0;
                piso_bg_low_ = 0;
                fetch_x_ = 0;
                scanline_++;
                cur_y_++;
                fetch_secondary_oam();
            }
            fetch_sprite_data();
        }
        else if (scanline_cycle_ <= 336)
        {
            oam_addr_ = 0;
            fetch_y_ = cur_y_ / 8;
            execute_pipeline();
            pixel_cycle_++;
            if (pixel_cycle_ == 8)
            {
                pixel_cycle_ = 0;
                if (scanline_cycle_ == 328)
                {
                    fetch_x_ += 1;
                    piso_bg_high_ <<= 8;
                    piso_bg_low_ <<= 8;
                }
            }
        }
        else if (scanline_cycle_ <= 340)
        {
            oam_addr_ = 0;
        }
        else
        {
            oam_addr_ = 0;
            scanline_cycle_ = -1;
        }
    }

    void PPU::handle_empty_scanline()
    {
        if (scanline_cycle_ == 340)
        {
            scanline_++;
            scanline_cycle_ = 0;
            cur_y_++;
        }
    }

    void PPU::handle_prerender_scanline()
    {
        if (scanline_cycle_ == 9)
        {
            // Clear sprite 0 and vblank
            ppu_status_ &= ~0b1100'0000;
        }
        else if (scanline_cycle_ >= 321 && scanline_cycle_ <= 336)
        {
            fetch_x_ = 0;
            fetch_y_ = 0;
            scanline_ = 0;
            execute_pipeline();
            scanline_ = 261;
            pixel_cycle_++;
            if (pixel_cycle_ == 8)
            {
                pixel_cycle_ = 0;
                if (scanline_cycle_ == 328)
                {
                    fetch_x_ += 1;
                    piso_bg_high_ <<= 8;
                    piso_bg_low_ <<= 8;
                }
            }
        }
        else if (scanline_cycle_ == 340)
        {
            scanline_ = 0;
            cur_y_ = 0;
            scanline_cycle_ = -1;
        }
    }

    uint8_t* PPU::GetScreenData()
    {
        return screen_color_data_.data();
    }

    void PPU::execute_pipeline()
    {
        switch (pixel_cycle_)
        {
            case NT_BYTE_HIGH:
                nt_latch_ = fetch_nt();
                break;
            case AT_BYTE_HIGH:
                at_latch_ = fetch_at();
                break;
            case LOW_BG_BYTE_HIGH:
                pt_low_latch_ = fetch_pt_low();
                break;
            case HIGH_BG_BYTE_HIGH:
                pt_high_latch_ = fetch_pt_high();
                piso_bg_high_ |= pt_high_latch_;
                piso_bg_low_ |= pt_low_latch_;
                piso_at_ = at_latch_;
                break;
            default:
                break;
        }
    }

    void PPU::invalidate(uint8_t addr, uint8_t data)
    {
        switch (addr & 0b111)
        {
            case 0b000:
            {
                ppu_ctrl_ = data;
                nt_addr_ = 0x2000 + (vram_addr_ & ~0x0C00) | ((ppu_ctrl_ & 0b11) << 10);
                vram_incr_vertical_ = ppu_ctrl_ & 0b100;
                sprite_pattern_address_ = !!(ppu_ctrl_ & 0b1000) * 0x1000;
                background_pattern_address_ = !!(ppu_ctrl_ & 0b1'0000) * 0x1000;
                sprite_size_ = ppu_ctrl_ & 0b10'0000;
                ppu_master_ = ppu_ctrl_ & 0b100'0000;
                nmi_output_ = ppu_ctrl_ & 0b1000'0000;
                break;
            }
            case 0b001:
            {
                ppu_mask_ = data;
                break;
            }
            case 0b010:
            {
                ppu_status_ = data;
                break;
            }
            case 0b011:
            {
                oam_addr_ = data;
                break;
            }
            case 0b100:
            {
                oam_data_ = data;
                auto sprite = oam_addr_ / 4;
                auto attr = oam_addr_ % 4;
                switch (attr)
                {
                    case 0:
                        oam_[sprite].y = data;
                        break;
                    case 1:
                        oam_[sprite].tile_index = data;
                        break;
                    case 2:
                        oam_[sprite].attributes = data;
                        break;
                    case 3:
                        oam_[sprite].x = data;
                        break;
                }
                oam_addr_++;
                break;
            }
            case 0b101:
            {
                if (scanline_ >= 241 && scanline_ <= 260)
                {
                    Logger::Warn("Writing to PPU scroll during vblank");
                }

                if (!write_toggle_)
                {
                    fine_x_ = data & 0b111;
                    vram_addr_latch_ = (vram_addr_latch_ & ~0x1F) | (data >> 3);
                }
                else
                {
                    vram_addr_latch_ =
                        (vram_addr_latch_ & ~0x73E0) | ((data & 0xF8) << 2) | ((data & 0x07) << 12);
                }
                write_toggle_ = !write_toggle_;
                break;
            }
            case 0b110:
            {
                if (!write_toggle_)
                {
                    vram_addr_latch_ = (vram_addr_latch_ & 0xFF) | ((data & 0x3F) << 8);
                }
                else
                {
                    vram_addr_latch_ = (vram_addr_latch_ & 0xFF00) | data;
                    vram_addr_ = vram_addr_latch_;
                }
                write_toggle_ = !write_toggle_;
                break;
            }
            case 0b111:
            {
                write(vram_addr_, data);
                vram_addr_ += 1 + 31 * vram_incr_vertical_;
                break;
            }
        }
        open_bus_ = data;
    }

    void PPU::dma(std::array<uint8_t, 256> data)
    {
        for (int i = 0; i < 64; i++)
        {
            Sprite sprite;
            sprite.y = data[i * 4];
            sprite.tile_index = data[i * 4 + 1];
            sprite.attributes = data[i * 4 + 2];
            sprite.x = data[i * 4 + 3];
            oam_[i] = sprite;
        }
    }

    uint8_t PPU::fetch_nt()
    {
        auto addr = (nt_addr_ + fetch_x_ + fetch_y_ * 32) & 0x7FF;
        return vram_.at(addr);
    }

    uint8_t PPU::fetch_at()
    {
        auto addr = ((nt_addr_ + 0x3C0) + ((fetch_y_ / 4) * 8) + (fetch_x_ / 4)) & 0x7FF;
        return vram_.at(addr);
    }

    uint8_t PPU::fetch_pt_low()
    {
        uint16_t addr = nt_latch_ * 16 + (scanline_ & 0b111);
        return read(background_pattern_address_ + addr);
    }

    uint8_t PPU::fetch_pt_high()
    {
        uint16_t addr = nt_latch_ * 16 + (scanline_ & 0b111);
        return read(background_pattern_address_ + 8 + addr);
    }

    void PPU::fetch_secondary_oam()
    {
        int j = 0;
        for (int i = 0; i < 64; i++)
        {
            Sprite sprite = oam_[i];
            if (sprite.y <= scanline_ && sprite.y + 8 > scanline_)
            {
                secondary_oam_[j] = sprite;
                j++;
                if (j == 8)
                {
                    break;
                }
            }
        }
        scanline_sprite_count_ = j;
        for (int i = 0; i < 8; i++)
        {
            sprite_shift_registers_[i].first = 0;
            sprite_shift_registers_[i].second = 0;
            attribute_latches_[i] = 0;
            sprite_counters_[i] = 0;
            sprite_active_[i] = false;
        }
    }

    void PPU::fetch_sprite_data()
    {
        auto current_cycle = scanline_cycle_ - 257;
        auto current_sprite = current_cycle / 8;
        if (current_sprite >= scanline_sprite_count_)
        {
            return;
        }
        bool flip_x = attribute_latches_[current_sprite] & 0b0100'0000;
        // bool flip_y = attribute_latches_[current_sprite] & 0b1000'0000;
        switch (current_cycle & 0b111)
        {
            case 5:
            {
                sprite_shift_registers_[current_sprite].first =
                    read(sprite_pattern_address_ + secondary_oam_[current_sprite].tile_index * 16 +
                         (scanline_ - secondary_oam_[current_sprite].y));
                if (flip_x)
                {
                    sprite_shift_registers_[current_sprite].first =
                        reverse(sprite_shift_registers_[current_sprite].first);
                }
                break;
            }
            case 7:
            {
                sprite_shift_registers_[current_sprite].second =
                    read(sprite_pattern_address_ + secondary_oam_[current_sprite].tile_index * 16 +
                         (scanline_ - secondary_oam_[current_sprite].y) + 8);
                if (flip_x)
                {
                    sprite_shift_registers_[current_sprite].second =
                        reverse(sprite_shift_registers_[current_sprite].second);
                }
                break;
            }
            case 3:
            {
                attribute_latches_[current_sprite] = secondary_oam_[current_sprite].attributes;
                sprite_counters_[current_sprite] = secondary_oam_[current_sprite].x;
                break;
            }
            default:
                break;
        }
    }

    void PPU::draw_pixel()
    {
        if ((vram_addr_ & 0x3F00) == 0x3F00)
        {
            printf("vram_addr: %x\n", vram_addr_);
            return;
        }

        uint8_t bg_cur = ((piso_bg_high_ >> 15) << 1) | (piso_bg_low_ >> 15);
        auto pixel = cur_x_ * 4 + cur_y_ * 256 * 4;
        uint8_t left_shift = !!(cur_x_ & 0b10000) * 2;
        uint8_t top_shift = !!(cur_y_ & 0b10000) * 4;
        piso_at_ = read(nt_addr_ + 0x3c0 + cur_x_ / 32 + (cur_y_ / 32) * 8);
        auto pal_index = ((piso_at_ >> left_shift) >> top_shift) & 0b11;
        uint8_t red = universal_bg_[0];
        uint8_t green = universal_bg_[1];
        uint8_t blue = universal_bg_[2];
        if (bg_cur != 0)
        {
            red = background_palettes_[pal_index][bg_cur][0];
            green = background_palettes_[pal_index][bg_cur][1];
            blue = background_palettes_[pal_index][bg_cur][2];
        }
        for (int i = 0; i < 8; i++)
        {
            if (sprite_counters_[i] != 0)
            {
                sprite_counters_[i]--;
            }
            if (sprite_counters_[i] == 0)
            {
                sprite_active_[i] = true;
            }
            if (sprite_active_[i])
            {
                uint8_t sprite_cur = ((sprite_shift_registers_[i].second >> 7) << 1) |
                                     (sprite_shift_registers_[i].first >> 7);
                if (sprite_cur != 0 && bg_cur != 0 && i == 0)
                {
                    ppu_status_ |= 0b0100'0000;
                }
                sprite_shift_registers_[i].first <<= 1;
                sprite_shift_registers_[i].second <<= 1;
                if (sprite_cur != 0)
                {
                    auto sprite_pal_index = attribute_latches_[i] & 0b11;
                    red = sprite_palettes_[sprite_pal_index][sprite_cur][0];
                    green = sprite_palettes_[sprite_pal_index][sprite_cur][1];
                    blue = sprite_palettes_[sprite_pal_index][sprite_cur][2];
                    continue;
                }
            }
        }
        if (draw_tile_grid_ || draw_metatile_grid_ || draw_attribute_grid_)
        {
            // TODO: add fine_x
            // TODO: check against single debug boolean (debugging on/off)
            if (draw_tile_grid_ && (cur_x_ % 8 == 0 || cur_y_ % 8 == 0))
            {
                red = 255;
                green = 0;
                blue = 0;
            }
            if (draw_metatile_grid_ && (cur_x_ % 16 == 0 || cur_y_ % 16 == 0))
            {
                red = 0;
                green = 255;
                blue = 0;
            }
            if (draw_attribute_grid_ && (cur_x_ % 32 == 0 || cur_y_ % 32 == 0))
            {
                red = 0;
                green = 0;
                blue = 255;
            }
        }
        screen_color_data_second_.at(pixel) = red;
        screen_color_data_second_.at(pixel + 1) = green;
        screen_color_data_second_.at(pixel + 2) = blue;
        screen_color_data_second_.at(pixel + 3) = 255;
        cur_x_++;
        piso_bg_low_ <<= 1;
        piso_bg_high_ <<= 1;
    }

    uint8_t PPU::read(uint16_t addr)
    {
        if (addr < 0x2000)
        {
            return chr_rom_.at(addr & 0x1FFF);
        }
        else if (addr < 0x3000)
        {
            return vram_.at(addr & 0x7FF);
        }
        else if (addr > 0x3F00 && addr < 0x3F20)
        {
            switch (addr & 0xFF)
            {
                case 0x00:
                case 0x10:
                {
                    return universal_bg_[0];
                }
                case 0x1:
                case 0x2:
                case 0x3:
                case 0x11:
                case 0x12:
                case 0x13:
                {
                    return master_palette_[addr & 0x1F][0];
                }
                case 0x4:
                case 0x5:
                case 0x6:
                case 0x14:
                case 0x15:
                case 0x16:
                {
                    return master_palette_[addr & 0x1F][1];
                }
                case 0x7:
                case 0x8:
                case 0x9:
                case 0x17:
                case 0x18:
                case 0x19:
                {
                    return master_palette_[addr & 0x1F][2];
                }
                case 0x0A:
                case 0x0B:
                case 0x0C:
                case 0x1A:
                case 0x1B:
                case 0x1C:
                {
                    return master_palette_[addr & 0x1F][3];
                }
                case 0x0D:
                case 0x0E:
                case 0x0F:
                case 0x1D:
                case 0x1E:
                case 0x1F:
                {
                    return master_palette_[addr & 0x1F][4];
                }
                default:
                    return 0;
            }
        }
        return 0;
    }

    void PPU::write(uint16_t addr, uint8_t data)
    {
        if (addr < 0x2000)
        {
            chr_rom_.at(addr) = data;
        }
        else if (addr < 0x3000)
        {
            vram_.at(addr & 0x7FF) = data;
        }
        else if (addr >= 0x3F00 && addr < 0x3F20)
        {
            data &= 0x3F;
            addr &= 0xFF;
            switch (addr)
            {
                case 0x00:
                case 0x10:
                {
                    universal_bg_[0] = master_palette_[data][0];
                    universal_bg_[1] = master_palette_[data][1];
                    universal_bg_[2] = master_palette_[data][2];
                    break;
                }
                case 0x1:
                case 0x2:
                case 0x3:
                case 0x5:
                case 0x6:
                case 0x7:
                case 0x9:
                case 0xA:
                case 0xB:
                case 0xD:
                case 0xE:
                case 0xF:
                {
                    auto cur = addr / 4;
                    auto col = (addr)&0b11;
                    background_palettes_.at(cur).at(col).at(0) = master_palette_[data][0];
                    background_palettes_.at(cur).at(col).at(1) = master_palette_[data][1];
                    background_palettes_.at(cur).at(col).at(2) = master_palette_[data][2];
                    break;
                }
                case 0x11:
                case 0x12:
                case 0x13:
                case 0x15:
                case 0x16:
                case 0x17:
                case 0x19:
                case 0x1A:
                case 0x1B:
                case 0x1D:
                case 0x1E:
                case 0x1F:
                {
                    auto cur = (addr & 0xF) / 4;
                    auto col = (addr)&0b11;
                    sprite_palettes_.at(cur).at(col).at(0) = master_palette_[data][0];
                    sprite_palettes_.at(cur).at(col).at(1) = master_palette_[data][1];
                    sprite_palettes_.at(cur).at(col).at(2) = master_palette_[data][2];
                    break;
                }
            }
        }
    }

    void PPU::Reset()
    {
        cur_x_ = 0;
        cur_y_ = 0;
        scanline_ = 0;
        scanline_cycle_ = 0;
        pixel_cycle_ = 0;
        for (int i = 0; i < 256 * 240 * 4; i += 4)
        {
            screen_color_data_[i] = 0;
            screen_color_data_[i + 1] = 0;
            screen_color_data_[i + 2] = 0;
            screen_color_data_[i + 3] = 255;
        }
        vram_.fill(0);
        write_toggle_ = false;
    }
} // namespace hydra::NES