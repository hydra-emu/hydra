#include <n64/qa/n64_angrylion_replayer.hxx>

#include <cstdarg>
#include <cstdio>

void vdac_init(struct n64video_config*) {}

void vdac_write(struct frame_buffer* fb)
{
    AngrylionReplayer::Current()->WriteFramebuffer(fb->pixels, fb->width, fb->height, fb->pitch);
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
    printf(buffer);
}

void msg_warning(const char* err, ...)
{
    va_list va;
    va_start(va, err);
    char buffer[16 * 1024];
    vsnprintf(buffer, sizeof(buffer), err, va);
    va_end(va);
    printf(buffer);
}

void msg_debug(const char* err, ...)
{
    va_list va;
    va_start(va, err);
    char buffer[16 * 1024];
    vsnprintf(buffer, sizeof(buffer), err, va);
    va_end(va);
    printf(buffer);
}

AngrylionReplayer::AngrylionReplayer()
{
    rdram_.resize(0x800000);
    config_.gfx.rdram = rdram_.data();
    config_.gfx.rdram_size = rdram_.size();
    config_.gfx.vi_reg = vi_regs_.data();
    config_.gfx.dp_reg = dp_regs_.data();
    config_.gfx.mi_intr_reg = &mi_interrupt_;
    config_.gfx.mi_intr_cb = []() {};
    config_.vi.mode = VI_MODE_NORMAL;
    config_.vi.interp = VI_INTERP_LINEAR;
    config_.dp.compat = DP_COMPAT_HIGH;
    n64video_init(&config_);
}

AngrylionReplayer::~AngrylionReplayer()
{
    n64video_close();
}

AngrylionReplayer* AngrylionReplayer::Current()
{
    static AngrylionReplayer replayer;
    return &replayer;
}

void AngrylionReplayer::WriteFramebuffer(const void* pixels, uint32_t width, uint32_t height,
                                         uint32_t pitch)
{
    printf("WriteFramebuffer: %p %d %d %d\n", pixels, width, height, pitch);
}