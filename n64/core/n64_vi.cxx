#include "n64_vi.hxx"
#include <fmt/format.h>
#include <log.hxx>

namespace hydra::N64
{
    void Vi::Reset()
    {
        vi_v_intr_ = 0x100;
    }

    bool Vi::Redraw()
    {
        width_ = vi_h_end_ - vi_h_start_;
        height_ = (vi_v_end_ - vi_v_start_) / 2;
        width_ *= vi_x_scale_ ? vi_x_scale_ : 512;
        height_ *= vi_y_scale_ ? vi_y_scale_ : 512;
        if (width_ == 0 || height_ == 0 || !memory_ptr_)
        {
            framebuffer_ptr_ = nullptr;
            return false;
        }
        width_ >>= 10;
        height_ >>= 10;
        size_t new_size = width_ * height_ * 4;
        if (framebuffer_.size() != new_size)
        {
            framebuffer_.resize(new_size);
        }
        switch (pixel_mode_)
        {
            case 0b11:
            {
                for (int y = 0; y < height_; y++)
                {
                    for (int x = 0; x < width_; x++)
                    {
                        uint32_t color =
                            (reinterpret_cast<uint32_t*>(memory_ptr_))[(y * vi_width_) + x];
                        set_pixel(x, y, color);
                    }
                }
                break;
            }
            case 0b10:
            {
                for (int y = 0; y < height_; y++)
                {
                    for (int x = 0; x < width_; x++)
                    {
                        uint16_t color_temp =
                            (reinterpret_cast<uint16_t*>(memory_ptr_))[(y * vi_width_) + x];
                        color_temp = (color_temp >> 8) | (color_temp << 8);
                        uint8_t r = (color_temp >> 11) & 0x1F;
                        uint8_t g = (color_temp >> 6) & 0x1F;
                        uint8_t b = (color_temp >> 1) & 0x1F;
                        r = (r << 3) | (r >> 2);
                        g = (g << 3) | (g >> 2);
                        b = (b << 3) | (b >> 2);
                        uint32_t color = 0xffu << 24 | b << 16 | g << 8 | r;
                        set_pixel(x, y, color);
                    }
                }
            }
            default:
                break;
        }
        framebuffer_ptr_ = framebuffer_.data();
        return true;
    }

    void Vi::set_pixel(int x, int y, uint32_t color)
    {
        memcpy(&framebuffer_[(x + y * width_) * 4], &color, 4);
    }
} // namespace hydra::N64