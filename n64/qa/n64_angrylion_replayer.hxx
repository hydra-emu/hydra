#pragma once

extern "C" {
#include "msg.h"
#include "n64video.h"
#include "vdac.h"
}
#include <array>
#include <cstdint>
#include <vector>

class AngrylionReplayer
{
public:
    AngrylionReplayer();
    ~AngrylionReplayer();

    static AngrylionReplayer* Current();
    void WriteFramebuffer(const void* pixels, uint32_t width, uint32_t height, uint32_t pitch);

private:
    n64video_config config_ = {};
    std::vector<uint8_t> rdram_;
    std::array<uint32_t*, VI_NUM_REG> vi_regs_ = {};
    std::array<uint32_t*, DP_NUM_REG> dp_regs_ = {};
    uint32_t mi_interrupt_ = 0;
};