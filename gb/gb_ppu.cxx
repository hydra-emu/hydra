#include <algorithm>
#include <gb/gb_ppu.hxx>
#include <iostream>
#define c(x) (((x) << 3) | ((x) >> 2))

namespace hydra::Gameboy
{
    enum STATMode {
        MODE_OAM_SCAN = 2,
        MODE_DRAW_PIXELS = 3,
        MODE_HBLANK = 0,
        MODE_VBLANK = 1,
    };

    uint8_t reverse(uint8_t b)
    {
        b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
        b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
        b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
        return b;
    }

    PPU::PPU(Bus& bus)
        : bus_(bus), LCDC(bus.GetReference(0xFF40)), STAT(bus.GetReference(0xFF41)),
          LYC(bus.GetReference(0xFF45)), LY(bus.GetReference(0xFF44)), IF(bus.GetReference(0xFF0F)),
          SCY(bus.GetReference(0xFF42)), SCX(bus.GetReference(0xFF43)),
          WY(bus.GetReference(0xFF4A)), WX(bus.GetReference(0xFF4B))
    {
        screen_color_data_.resize(4 * 160 * 144);
        screen_color_data_second_.resize(4 * 160 * 144);
        Reset();
    }

    void PPU::Update(uint8_t cycles)
    {
        static constexpr int clock_max = 456 * 144 + 456 * 10;
        static int mode3_extend = 0; // unused for now
        clock_ += cycles;
        clock_ %= clock_max;
        auto true_ly = clock_ / 456;
        if (LY != true_ly)
        {
            LY = true_ly;
            IF |= update_lyc();
        }
        if (LYC == LY)
        {
            STAT |= STATFlag::COINCIDENCE;
        }
        bool enabled = LCDC & LCDCFlag::LCD_ENABLE;
        if (LY <= 143)
        {
            // Normal scanline
            auto cur_scanline_clocks = clock_ % 456;
            // Every scanline takes 456 tclocks
            if (cur_scanline_clocks < 80)
            {
                // OAM scan
                bus_.CurScanlineX = -1;
                if (get_mode() != MODE_OAM_SCAN)
                {
                    if (enabled)
                        bus_.OAMAccessible = false;
                    // Load the 10 sprites for this line
                    cur_scanline_sprites_.clear();
                    mode3_extend = 0;
                    for (size_t i = 0; i < (bus_.oam_.size()); i += 4)
                    {
                        // SCX & 7 > 0
                        if (is_sprite_eligible(bus_.oam_[i]))
                        {
                            if (cur_scanline_sprites_.size() < 10)
                            {
                                cur_scanline_sprites_.push_back(i);
                            }
                            // Special behavior for x = 0, lengthens mode 2
                            if (bus_.oam_[i + 1] == 0)
                            {
                                mode3_extend += SCX & 7;
                            }
                        }
                    }
                    // Sprites for this scanline are now scanned
                    IF |= set_mode(MODE_OAM_SCAN);
                }
                // Scanline changes only matter during pixel draw
                bus_.ScanlineChanges.clear();
            } else if (cur_scanline_clocks < (80 + 172 + mode3_extend))
            {
                // TODO: don't really know why the -12 but it seems to pass mealybug test :)
                // Investigate? probably not needed if we impl fifo
                bus_.CurScanlineX = cur_scanline_clocks - 80 - 12;
                if (LY == 0)
                {
                    bus_.CurScanlineX += 4;
                }
                if (get_mode() != MODE_DRAW_PIXELS)
                {
                    IF |= set_mode(MODE_DRAW_PIXELS);
                }
            } else
            {
                if (get_mode() != MODE_HBLANK)
                {
                    if (enabled)
                        bus_.OAMAccessible = true;
                    auto mod = SCX % 8;
                    if (mod == 0)
                    {

                    } else if (mod <= 4)
                    {
                        clock_ += 4;
                    } else
                    {
                        clock_ += 8;
                    }
                    IF |= set_mode(MODE_HBLANK);
                    draw_scanline();
                    bus_.ScanlineChanges.clear();
                }
            }
        } else
        {
            // VBlank scanline
            if (get_mode() != MODE_VBLANK)
            {
                // VBlank interrupt triggers when we first enter vblank
                // and never again during vblank
                IF |= IFInterrupt::VBLANK;
                if (STAT & STATFlag::MODE2_INTER)
                {
                    // Only in DMG
                    IF |= IFInterrupt::LCDSTAT;
                }
                IF |= set_mode(MODE_VBLANK);
                window_internal_ = 0;
                window_internal_temp_ = 0;
                std::swap(screen_color_data_, screen_color_data_second_);
                ReadyToDraw = true;
            }
        }
        if (!enabled)
        {
            clock_ = 0;
            STAT &= 0b1111'1100;
            LY = 0;
        }
    }

    bool PPU::is_sprite_eligible(uint8_t sprite_y)
    {
        bool use8x16 = LCDC & LCDCFlag::OBJ_SIZE;
        int y_pos_end = sprite_y - (!use8x16 * 8);
        int y_pos_start = sprite_y - 16;
        // In order for this sprite to be one of the 10 drawn in this scanline
        // one of its 8 (or 16) lines need to be equal to LY
        if (LY >= y_pos_start && LY < y_pos_end)
        {
            return true;
        }
        return false;
    }

    void PPU::Reset()
    {
        for (size_t i = 0; i < 4; i++)
        {
            bus_.Palette[i][0] = 0xFF - 64 * i;
            bus_.Palette[i][1] = 0xFF - 64 * i;
            bus_.Palette[i][2] = 0xFF - 64 * i;
        }
        for (size_t i = 0; i < (screen_color_data_second_.size() - 4); i += 4)
        {
            screen_color_data_second_[i + 0] = bus_.Palette[0][0];
            screen_color_data_second_[i + 1] = bus_.Palette[0][1];
            screen_color_data_second_[i + 2] = bus_.Palette[0][2];
            screen_color_data_second_[i + 3] = 255;
        }
        for (size_t i = 0; i < (screen_color_data_.size() - 4); i += 4)
        {
            screen_color_data_[i + 0] = bus_.Palette[0][0];
            screen_color_data_[i + 1] = bus_.Palette[0][1];
            screen_color_data_[i + 2] = bus_.Palette[0][2];
            screen_color_data_[i + 3] = 255;
        }
        LY = 0x0;
        LCDC = 0b1001'0001;
        STAT = 0b1000'0000;
        clock_ = 0;
        clock_target_ = 0;
    }

    uint8_t* PPU::GetScreenData() { return &screen_color_data_[0]; }

    int PPU::set_mode(int mode)
    {
        mode &= 0b11;
        STAT &= 0b1111'1100;
        STAT |= mode;
        if (mode != 3 && (STAT & (1 << (mode + 3))))
        {
            return IFInterrupt::LCDSTAT;
        }
        return 0;
    }

    int PPU::get_mode() { return STAT & STATFlag::MODE; }

    int PPU::update_lyc()
    {
        if (LYC == LY)
        {
            STAT |= STATFlag::COINCIDENCE;
            if (STAT & STATFlag::COINC_INTER)
            {
                return IFInterrupt::LCDSTAT;
            }
        } else
        {
            STAT &= 0b1111'1011;
        }
        return 0;
    }

    void PPU::draw_scanline()
    {
        bool enabled = LCDC & LCDCFlag::LCD_ENABLE;
        if (enabled)
        {
            if (UseCGB)
            {
                render_tiles();
            } else if (LCDC & LCDCFlag::BG_ENABLE)
            {
                render_tiles();
            }
            if (LCDC & LCDCFlag::OBJ_ENABLE && DrawSprites)
            {
                render_sprites();
            }
        }
    }

    inline void PPU::render_tiles()
    {
        uint16_t tileData = (LCDC & LCDCFlag::BG_TILES) ? 0x8000 : 0x8800;
        bool unsig = true;
        if (tileData == 0x8800)
        {
            unsig = false;
        }
        bool windowEnabled = (LCDC & LCDCFlag::WND_ENABLE && WY <= LY);
        if (WX >= 166 || WX == 0)
        {
            windowEnabled = false;
        }
        uint16_t identifierLocationW = (LCDC & LCDCFlag::WND_TILEMAP) ? 0x9C00 : 0x9800;
        uint16_t identifierLocationB = (LCDC & LCDCFlag::BG_TILEMAP) ? 0x9C00 : 0x9800;
        uint8_t positionY = LY + SCY;
        if (windowEnabled)
        {
            ++window_internal_temp_;
        } else if (window_internal_temp_)
        {
            window_internal_ = window_internal_temp_ - 2;
        }
        uint16_t identifierLoc = identifierLocationB;
        uint16_t tileRow = (((uint8_t)(positionY / 8)) * 32);
        for (int pixel = 0; pixel < 160; pixel++)
        {
            uint8_t positionX = pixel + SCX;
            if (windowEnabled && pixel >= (WX - 7))
            {
                identifierLoc = identifierLocationW;
                positionX = pixel - (WX - 7);
                positionY = LY - WY - (window_internal_ * 4);
                tileRow = (((uint8_t)(positionY / 8)) * 32);
            }
            uint16_t tileCol = (positionX / 8);
            int16_t tileNumber;
            uint16_t tileAddress = identifierLoc + tileRow + tileCol;
            uint16_t tileLocation = tileData;
            if (unsig)
            {
                tileNumber = bus_.Read(tileAddress);
                tileLocation += tileNumber * 16;
            } else
            {
                tileNumber = static_cast<int8_t>(bus_.Read(tileAddress));
                tileLocation += (tileNumber + 128) * 16;
            }
            uint8_t line = (positionY % 8) * 2;
            uint8_t attrib = bus_.vram_banks_[1][(identifierLoc + tileRow + tileCol) % 0x2000];
            bool vram_banks_bank = UseCGB ? (attrib & 0b1000) : false;
            bool xFlip = UseCGB ? (attrib & 0b100000) : false;
            bool yFlip = UseCGB ? (attrib & 0b1000000) : false;
            if (yFlip)
            {
                line = 14 - line;
            }
            uint8_t data1 = bus_.vram_banks_[vram_banks_bank][(tileLocation + line) % 0x2000];
            uint8_t data2 = bus_.vram_banks_[vram_banks_bank][(tileLocation + line + 1) % 0x2000];
            if (xFlip)
            {
                data1 = reverse(data1);
                data2 = reverse(data2);
            }
            int colorBit = -((positionX % 8) - 7);
            int colorNum = (((data2 >> colorBit) & 0b1) << 1) | ((data1 >> colorBit) & 0b1);
            int idx = (pixel * 4) + (LY * 4 * 160);
            PaletteColors& bg_ref = UseCGB ? get_cur_bg_pal(attrib & 0b111) : get_cur_bg_pal(0);
            uint8_t red, green, blue;
            if (UseCGB)
            {
                red = c(bg_ref[colorNum] & 0b11111);
                green = c((bg_ref[colorNum] >> 5) & 0b11111);
                blue = c((bg_ref[colorNum] >> 10) & 0b11111);
            } else
            {
                red = bus_.Palette[bg_ref[colorNum]][0];
                green = bus_.Palette[bg_ref[colorNum]][1];
                blue = bus_.Palette[bg_ref[colorNum]][2];
            }
            if (windowEnabled && identifierLoc == identifierLocationW)
            {
                if (!DrawWindow)
                {
                    screen_color_data_second_[idx++] = bus_.Palette[0][0];
                    screen_color_data_second_[idx++] = bus_.Palette[0][1];
                    screen_color_data_second_[idx++] = bus_.Palette[0][2];
                    screen_color_data_second_[idx] = 255;
                    continue;
                }
            } else if (!DrawBackground)
            {
                screen_color_data_second_[idx++] = bus_.Palette[0][0];
                screen_color_data_second_[idx++] = bus_.Palette[0][1];
                screen_color_data_second_[idx++] = bus_.Palette[0][2];
                screen_color_data_second_[idx] = 255;
                continue;
            }
            if (bus_.ScanlineChanges.size() > 0) [[unlikely]]
            {
                bool has_change = bus_.ScanlineChanges.contains(pixel);
                if (has_change) [[unlikely]]
                {
                    // This sets it to the changed palette if it exists, or to itself if it doesn't
                    bg_ref = std::move(bus_.ScanlineChanges[pixel].new_bg_pal.value_or(bg_ref));
                }
            }
            screen_color_data_second_[idx++] = red;
            screen_color_data_second_[idx++] = green;
            screen_color_data_second_[idx++] = blue;
            screen_color_data_second_[idx] = 255;
        }
    }

    void PPU::render_sprites()
    {
        bool use8x16 = LCDC & LCDCFlag::OBJ_SIZE;
        // Sort depending on X and reverse iterate to correctly select sprite priority
        if (!UseCGB)
        {
            std::sort(cur_scanline_sprites_.begin(), cur_scanline_sprites_.end(),
                      [this](const auto& lhs, const auto& rhs) {
                          return bus_.oam_[lhs + 1] < bus_.oam_[rhs + 1];
                      });
        }
        for (auto i = cur_scanline_sprites_.rbegin(); i != cur_scanline_sprites_.rend(); ++i)
        {
            auto sprite = *i;
            int16_t positionY = bus_.oam_[sprite] - 16;
            int16_t positionX = bus_.oam_[sprite + 1] - 8;
            uint8_t tileLoc = bus_.oam_[sprite + 2];
            uint8_t attributes = bus_.oam_[sprite + 3];
            if (use8x16)
            {
                // dmg-acid2: Bit 0 of tile index for 8x16 objects should be ignored
                tileLoc &= 0b1111'1110;
            }
            bool yFlip = attributes & 0b1000000;
            bool xFlip = attributes & 0b100000;
            // if its 8 aligned the sprite cant be between two tiles so we dont
            // need to fetch the attributes every pixel
            int height = use8x16 ? 16 : 8;
            int line = LY - positionY;
            if (yFlip)
            {
                line -= height - 1;
                line *= -1;
            }
            line *= 2;
            uint16_t address = (0x8000 + (tileLoc * 16) + line);
            bool vram_banks_bank = UseCGB ? (attributes & 0b1000) : false;
            uint8_t data1 = bus_.vram_banks_[vram_banks_bank][address % 0x2000];
            uint8_t data2 = bus_.vram_banks_[vram_banks_bank][(address + 1) % 0x2000];
            for (int tilePixel = 7; tilePixel >= 0; tilePixel--)
            {
                int colorbit = tilePixel;
                if (xFlip)
                {
                    colorbit -= 7;
                    colorbit *= -1;
                }
                int colorNum = ((data2 >> colorbit) & 0b1) << 1;
                colorNum |= (data1 >> colorbit) & 0b1;
                bool obp1 = (attributes & 0b10000);
                auto& obj_ref =
                    UseCGB ? get_cur_obj_pal(attributes & 0b111) : get_cur_obj_pal(obp1);
                int pixel = positionX - tilePixel + 7;
                if ((LY > 143) || (pixel < 0) || (pixel > 159) || (colorNum == 0))
                {
                    continue;
                }
                int idx = (pixel * 4) + (LY * 4 * 160);
                bool windowEnabled =
                    (LCDC & LCDCFlag::WND_ENABLE && WY <= LY) && positionX >= (WX - 7);
                uint16_t identifierLoc;
                if (windowEnabled)
                {
                    uint16_t identifierLocationW = (LCDC & LCDCFlag::WND_TILEMAP) ? 0x9C00 : 0x9800;
                    identifierLoc = identifierLocationW;
                } else
                {
                    uint16_t identifierLocationB = (LCDC & LCDCFlag::BG_TILEMAP) ? 0x9C00 : 0x9800;
                    identifierLoc = identifierLocationB;
                }
                uint16_t bg_offset = ((SCX + pixel) / 8) + (((SCY + LY) / 8) * 32);
                if (windowEnabled)
                {
                    bg_offset =
                        ((pixel - (WX - 7)) / 8) + ((LY - WY - (window_internal_ * 4)) / 8) * 32;
                }
                uint8_t bg_attributes = bus_.vram_banks_[1][(identifierLoc + bg_offset) % 0x2000];
                bool master_priority = LCDC & LCDCFlag::BG_ENABLE;
                uint8_t red, green, blue;
                if (UseCGB)
                {
                    red = c(obj_ref[colorNum] & 0b11111);
                    green = c((obj_ref[colorNum] >> 5) & 0b11111);
                    blue = c((obj_ref[colorNum] >> 10) & 0b11111);
                } else
                {
                    auto color = obj_ref[colorNum];
                    red = bus_.Palette[color][0];
                    green = bus_.Palette[color][1];
                    blue = bus_.Palette[color][2];
                }
                if (UseCGB)
                {
                    if ((bg_attributes & 0b1000'0000) && master_priority)
                    {
                        auto& bg_ref = get_cur_bg_pal(bg_attributes & 0b111);
                        auto bg_red = c(bg_ref[0] & 0b11111);
                        auto bg_green = c((bg_ref[0] >> 5) & 0b11111);
                        auto bg_blue = c((bg_ref[0] >> 10) & 0b11111);
                        if (!(screen_color_data_second_[idx] == bg_red &&
                              screen_color_data_second_[idx + 1] == bg_green &&
                              screen_color_data_second_[idx + 2] == bg_blue))
                        {
                            continue;
                        }
                    }
                }
                if (attributes & 0b1000'0000)
                {
                    if (UseCGB)
                    {
                        if (master_priority)
                        {
                            auto& bg_ref = get_cur_bg_pal(bg_attributes & 0b111);
                            auto bg_red = c(bg_ref[0] & 0b11111);
                            auto bg_green = c((bg_ref[0] >> 5) & 0b11111);
                            auto bg_blue = c((bg_ref[0] >> 10) & 0b11111);
                            if (!(screen_color_data_second_[idx] == bg_red &&
                                  screen_color_data_second_[idx + 1] == bg_green &&
                                  screen_color_data_second_[idx + 2] == bg_blue))
                            {
                                continue;
                            }
                        }
                    } else
                    {
                        auto& bg_ref = get_cur_bg_pal(0);
                        if (!(screen_color_data_second_[idx] == bus_.Palette[bg_ref[0]][0] &&
                              screen_color_data_second_[idx + 1] == bus_.Palette[bg_ref[0]][1] &&
                              screen_color_data_second_[idx + 2] == bus_.Palette[bg_ref[0]][2]))
                        {
                            continue;
                        }
                    }
                }
                red += (280 - red) * SpriteDebugColor;
                screen_color_data_second_[idx++] = red;
                screen_color_data_second_[idx++] = green;
                screen_color_data_second_[idx++] = blue;
                screen_color_data_second_[idx] = 255;
            }
        }
    }

    void PPU::FillTileset(float* pixels, size_t x_off, size_t y_off, uint16_t addr)
    {
        for (int y_ = 0; y_ < 16; ++y_)
        {
            for (int x_ = 0; x_ < 16; ++x_)
            {
                for (size_t i = 0; i < 16; i += 2)
                {
                    uint16_t curr_addr = addr + i + x_ * 16 + y_ * 256;
                    uint8_t data1 = bus_.Read(curr_addr);
                    uint8_t data2 = bus_.Read(curr_addr + 1);
                    int x = ((i / 16) + x_) * 8;
                    int y = i / 2 + y_ * 8;
                    size_t start_idx = y * 256 * 4 + x * 4 + x_off * 4 + y_off * 256 * 4;
                    for (size_t j = 0; j < 8; j++)
                    {
                        int color = (((data1 >> (7 - j)) & 0b1) << 1) + ((data2 >> (7 - j)) & 0b1);
                        pixels[start_idx + (j * 4) + 0] = bus_.Palette[color][0];
                        pixels[start_idx + (j * 4) + 1] = bus_.Palette[color][1];
                        pixels[start_idx + (j * 4) + 2] = bus_.Palette[color][2];
                        pixels[start_idx + (j * 4) + 3] = 255.0f;
                    }
                }
            }
        }
    }

    PaletteColors& PPU::get_cur_bg_pal(uint8_t attributes)
    {
        if (UseCGB)
        {
            return bus_.BGPalettes[attributes];
        } else
        {
            return bus_.BGPalettes[0];
        }
    }

    PaletteColors& PPU::get_cur_obj_pal(uint8_t attributes)
    {
        if (UseCGB)
        {
            return bus_.OBJPalettes[attributes];
        } else
        {
            return bus_.OBJPalettes[!!(attributes)];
        }
    }
} // namespace hydra::Gameboy
