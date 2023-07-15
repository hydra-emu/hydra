#include <filesystem>
#include <gtest/gtest.h>
#include <n64/core/n64_rdp.hxx>
#include <n64/core/n64_rdp_commands.hxx>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.hxx"

using namespace hydra::N64;

class RDPTest : public testing::Test
{
protected:
    void SetUp() override
    {
        std::fill(framebuffer.begin(), framebuffer.end(), 0);
        rdp.InstallBuses(framebuffer.data(), nullptr);

        SetColorImageCommand color_image;
        color_image.dram_address = 0;
        color_image.width = my_width - 1;
        color_image.format = 0;
        color_image.size = 3;
        rdp.SendCommand({color_image.full});

        SetFillColorCommand fill_color;
        fill_color.color = 0xffffffff;
        rdp.SendCommand({fill_color.full});

        SetOtherModesCommand other_modes;
        other_modes.cycle_type = 3;
        rdp.SendCommand({other_modes.full});

        SetScissorCommand scissor;
        scissor.XH = 0;
        scissor.YH = 0;
        scissor.XL = my_width << 2;
        scissor.YL = my_height << 2;
        rdp.SendCommand({scissor.full});
    }

    void TearDown() override {}

    void VerifyFramebuffer(std::string path)
    {
        if (!std::filesystem::exists(path))
        {
            FAIL();
        }

        int width, height, channels;
        stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, my_channels);

        if (data == nullptr)
        {
            FAIL();
        }

        if (my_width != width || my_height != height || my_channels != channels)
        {
            FAIL();
        }

        for (size_t i = 0; i < framebuffer.size(); i++)
        {
            if (data[i] != framebuffer[i])
            {
                FAIL();
            }
        }
    }

    void DumpPng(std::string path)
    {
        stbi_write_png(path.c_str(), my_width, my_height, my_channels, framebuffer.data(), 0);
    }

    void PrintCommand(const std::vector<uint64_t> triangle)
    {
        EdgeCoefficientsCommand command;
        command.full = triangle[0];
        EdgeCoefficients edgel, edgeh, edgem;
        edgel.full = triangle[1];
        edgeh.full = triangle[2];
        edgem.full = triangle[3];

        int32_t yh = static_cast<int16_t>(command.YH << 2) >> 4;
        int32_t ym = static_cast<int16_t>(command.YM << 2) >> 4;
        int32_t yl = static_cast<int16_t>(command.YL << 2) >> 4;
        int32_t slopel = edgel.slope;
        int32_t slopem = edgem.slope;
        int32_t slopeh = edgeh.slope;
        int32_t xl = edgel.X - slopel;
        int32_t xm = edgem.X;
        int32_t xh = edgeh.X;

        printf("yh: %d ym: %d yl: %d\n", yh, ym, yl);
        printf("xl: %d xm: %d xh: %d\n", xl >> 16, xm >> 16, xh >> 16);
        printf("slopel: %d.%d slopem: %d.%d slopeh: %d.%d\n", slopel >> 16, slopel & 0xffff,
               slopem >> 16, slopem & 0xffff, slopeh >> 16, slopeh & 0xffff);
    }

    static constexpr int my_width = 320;
    static constexpr int my_height = 240;
    static constexpr int my_channels = 4;

    std::array<uint8_t, my_width * my_height * my_channels> framebuffer;
    RDP rdp;
};

#define TRIANGLE_TEST(name, ...)                        \
    TEST_F(RDPTest, name)                               \
    {                                                   \
        std::vector<uint64_t> triangle = {__VA_ARGS__}; \
        rdp.SendCommand(triangle);                      \
        DumpPng("/tmp/" #name ".png");                  \
        VerifyFramebuffer("data/" #name ".png");        \
    }

TRIANGLE_TEST(Simple_Triangle, 0x088002a801180118, 0x00d20000ffff0000, 0x006e000000000000,
              0x006e000000000000);

TRIANGLE_TEST(Simple_Triangle_Flipped, 0x080002a801180118, 0x006e000000010000, 0x00d2000000000000,
              0x00d2000000000000);

TRIANGLE_TEST(Simple_Triangle_Using_YM, 0x08800300016c0118, 0x00d20000ffff0000, 0x006e000000000000,
              0x006e00000004c000);

TRIANGLE_TEST(Slopes_L_H_Intersect_Before_YM_YL, 0x088002a8016800f0, 0x00c80000ffff0000,
              0x005a000000010000, 0x00780000ffff0000);

TRIANGLE_TEST(Slopes_L_M_Intersect, 0x088002a8011800a0, 0x00d20000ffff0000, 0x006e00000000e000,
              0x006e00000004a000);

TRIANGLE_TEST(Slopes_L_M_Intersect_Flipped, 0x080002a8011800a0, 0x006e000000010000,
              0x00d20000ffff8000, 0x00d20000fffb4000);

TRIANGLE_TEST(Same_XH_XM_XL, 0x088002bc02bc0258, 0x00e1000000000000, 0x00e10000fffe0000,
              0x00e1000000000000);

int main()
{
    testing::InitGoogleTest();
    int result = RUN_ALL_TESTS();
    return result;
}