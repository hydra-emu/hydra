#include "n64_rcp.hxx"

namespace hydra::N64 {
    void RCP::Reset() {
        // These default values are in little endian
        rsp_.Reset();
        rdp_.Reset();
        vi_.Reset();
    }

    void RCP::SetPixelMode(uint8_t mode) {
        vi_.pixel_mode_ = mode;
    }

    void RCP::SetHVideo(uint32_t hvideo) {
        vi_.vi_h_end_ = hvideo & 0b11'1111'1111;
        vi_.vi_h_start_ = (hvideo >> 16) & 0b11'1111'1111;
    }

    void RCP::SetVVideo(uint32_t vvideo) {
        vi_.vi_v_end_ = vvideo & 0b11'1111'1111;
        vi_.vi_v_start_ = (vvideo >> 16) & 0b11'1111'1111;
    }

    // TODO: offsets
    void RCP::SetXScale(uint32_t xscale) {
        vi_.vi_x_scale_ = xscale & 0b1111'1111'1111;
    }

    void RCP::SetYScale(uint32_t yscale) {
        vi_.vi_y_scale_ = yscale & 0b1111'1111'1111;
    }

    bool RCP::Redraw() {
        return vi_.Redraw();
    }
}