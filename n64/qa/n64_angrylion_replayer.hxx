#pragma once

#include <cstdint>
#include <memory>
#include <vector>

class AngrylionReplayerImpl;
class frame_buffer;

struct Framebuffer
{
    struct rgba
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };

    std::vector<rgba> pixels;
    uint32_t width;
    uint32_t height;
};

struct AngrylionReplayer
{
    static void Init();
    static void RunCommand(const std::vector<uint64_t>& command);
    static Framebuffer GetFramebuffer();
    static void Cleanup();

    static std::unique_ptr<AngrylionReplayerImpl> impl_;
    static Framebuffer framebuffer_;
};