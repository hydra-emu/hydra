#include <n64/qa/n64_angrylion_replayer.hxx>

extern "C" {
#include "msg.h"
#include "n64video.h"
#include "vdac.h"
void rdp_cmd(uint32_t wid, const uint32_t* args);
}
#include <array>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

class AngrylionReplayerImpl
{
public:
    AngrylionReplayerImpl();
    ~AngrylionReplayerImpl();

private:
    n64video_config config_ = {};
    std::vector<uint8_t> rdram_;
    std::array<uint32_t*, VI_NUM_REG> vi_regs_ = {};
    std::array<uint32_t*, DP_NUM_REG> dp_regs_ = {};
    uint32_t mi_interrupt_ = 0;
};

std::unique_ptr<AngrylionReplayerImpl> AngrylionReplayer::impl_;
Framebuffer AngrylionReplayer::framebuffer_;

void AngrylionReplayer::Init()
{
    AngrylionReplayer::impl_ = std::make_unique<AngrylionReplayerImpl>();
}

void AngrylionReplayer::RunCommand(const std::vector<uint64_t>& command)
{
    std::vector<uint32_t> command32(command.size() * 2);
    for (size_t i = 0; i < command.size(); ++i)
    {
        command32[i * 2] = static_cast<uint32_t>(command[i] >> 32);
        command32[i * 2 + 1] = static_cast<uint32_t>(command[i]);
    }
    rdp_cmd(0, command32.data());
}

Framebuffer AngrylionReplayer::GetFramebuffer()
{
    n64video_update_screen();
    printf("test\n");
    return framebuffer_;
}

void AngrylionReplayer::Cleanup()
{
    AngrylionReplayer::impl_.reset();
}

void vdac_init(struct n64video_config*) {}

void vdac_write(struct frame_buffer* fb)
{
    AngrylionReplayer::framebuffer_.pixels.resize(fb->width * fb->height);
    uint8_t* src = (uint8_t*)fb->pixels;
    uint8_t* dst = (uint8_t*)AngrylionReplayer::framebuffer_.pixels.data();
    for (unsigned y = 0; y < fb->height; y++, src += fb->pitch * 4, dst += fb->width * 4)
        memcpy(dst, src, fb->width * sizeof(uint32_t));
    printf("vdac_write: %d %d\n", fb->width, fb->height);
}

void vdac_sync(bool invalid)
{
    if (invalid)
    {
        printf("vdac_sync: invalid\n");
    }
}

void vdac_close() {}

void msg_error(const char* err, ...)
{
    va_list va;
    va_start(va, err);
    char buffer[16 * 1024];
    vsnprintf(buffer, sizeof(buffer), err, va);
    va_end(va);
    printf("%s\n", buffer);
}

void msg_warning(const char* err, ...)
{
    va_list va;
    va_start(va, err);
    char buffer[16 * 1024];
    vsnprintf(buffer, sizeof(buffer), err, va);
    va_end(va);
    printf("%s\n", buffer);
}

void msg_debug(const char* err, ...)
{
    va_list va;
    va_start(va, err);
    char buffer[16 * 1024];
    vsnprintf(buffer, sizeof(buffer), err, va);
    va_end(va);
    printf("%s\n", buffer);
}

AngrylionReplayerImpl::AngrylionReplayerImpl()
{
    rdram_.resize(0x800000);
    for (auto& vi_reg : vi_regs_)
    {
        vi_reg = new uint32_t;
    }
    for (auto& dp_reg : dp_regs_)
    {
        dp_reg = new uint32_t;
    }
    config_.gfx.rdram = rdram_.data();
    config_.gfx.rdram_size = rdram_.size();
    config_.gfx.vi_reg = vi_regs_.data();
    config_.gfx.dp_reg = dp_regs_.data();
    config_.gfx.mi_intr_reg = &mi_interrupt_;
    config_.gfx.mi_intr_cb = []() {};
    config_.vi.mode = VI_MODE_NORMAL;
    config_.vi.interp = VI_INTERP_NEAREST;
    config_.dp.compat = DP_COMPAT_HIGH;

    *vi_regs_[VI_ORIGIN] = 0xa0100000;
    *vi_regs_[VI_WIDTH] = 320;
    *vi_regs_[VI_STATUS] = 0x00003303;
    *vi_regs_[VI_V_SYNC] = 0x20d;
    *vi_regs_[VI_H_START] = 0x006c02ec;
    *vi_regs_[VI_V_START] = 0x002501ff;
    *vi_regs_[VI_X_SCALE] = 0x200;
    *vi_regs_[VI_Y_SCALE] = 0x400;

    n64video_init(&config_);
}

AngrylionReplayerImpl::~AngrylionReplayerImpl()
{
    n64video_close();
}
