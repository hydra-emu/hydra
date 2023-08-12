#pragma once

#include <cstring>
#include <n64/core/n64_types.hxx>
#include <vector>

#define RDP_COMMANDS                      \
    X(Triangle, 0x8, 4)                   \
    X(TriangleDepth, 0x9, 6)              \
    X(TriangleTexture, 0xA, 12)           \
    X(TriangleTextureDepth, 0xB, 14)      \
    X(TriangleShade, 0xC, 12)             \
    X(TriangleShadeDepth, 0xD, 14)        \
    X(TriangleShadeTexture, 0xE, 20)      \
    X(TriangleShadeTextureDepth, 0xF, 22) \
    X(SyncPipe, 0x27, 1)                  \
    X(SyncFull, 0x29, 1)                  \
    X(SetScissor, 0x2D, 1)                \
    X(SetOtherModes, 0x2F, 1)             \
    X(SetBlendColor, 0x39, 1)             \
    X(SetColorImage, 0x3F, 1)             \
    X(SetFillColor, 0x37, 1)              \
    X(SetPrimitiveColor, 0x3A, 1)         \
    X(Rectangle, 0x36, 1)                 \
    X(SyncLoad, 0x26, 1)                  \
    X(TextureRectangle, 0x24, 2)          \
    X(SetTile, 0x35, 1)                   \
    X(LoadTile, 0x34, 1)                  \
    X(LoadTLUT, 0x30, 1)                  \
    X(SetTileSize, 0x32, 1)               \
    X(LoadBlock, 0x33, 1)                 \
    X(SetTextureImage, 0x3D, 1)           \
    X(SyncTile, 0x28, 1)                  \
    X(TextureRectangleFlip, 0x25, 2)      \
    X(SetPrimDepth, 0x2E, 1)              \
    X(SetZImage, 0x3E, 1)                 \
    X(SetCombineMode, 0x3C, 1)            \
    X(SetEnvironmentColor, 0x3B, 1)       \
    X(SetFogColor, 0x38, 1)

class N64Debugger;
class MmioViewer;

namespace hydra::N64
{
    class RSP;

    enum class RDPCommandType {
#define X(name, opcode, length) name = opcode,
        RDP_COMMANDS
#undef X
    };

    union RDPStatus
    {
        uint32_t full;

        struct
        {
            uint32_t dma_source_dmem : 1;
            uint32_t freeze          : 1;
            uint32_t flush           : 1;
            uint32_t start_gclk      : 1;
            uint32_t tmem_busy       : 1;
            uint32_t pipe_busy       : 1;
            uint32_t cmd_busy        : 1;
            uint32_t ready           : 1;
            uint32_t dma_busy        : 1;
            uint32_t end_pending     : 1;
            uint32_t start_pending   : 1;
            uint32_t                 : 21;
        };
    };

    static_assert(sizeof(RDPStatus) == sizeof(uint32_t));

    union RDPStatusWrite
    {
        uint32_t full;

        struct
        {
            uint32_t clear_dma_source_dmem : 1;
            uint32_t set_dma_source_dmem   : 1;
            uint32_t clear_freeze          : 1;
            uint32_t set_freeze            : 1;
            uint32_t clear_flush           : 1;
            uint32_t set_flush             : 1;
            uint32_t clear_tmem_busy       : 1;
            uint32_t clear_pipe_busy       : 1;
            uint32_t clear_buffer_busy     : 1;
            uint32_t clear_clock_busy      : 1;
            uint32_t                       : 22;
        };
    };

    static_assert(sizeof(RDPStatusWrite) == sizeof(uint32_t));

    enum class Format { RGBA, YUV, CI, IA, I };

    struct TileDescriptor
    {
        uint16_t tmem_address;
        Format format;
        uint8_t size;
        uint8_t palette_index;
        uint16_t line_width;
        uint16_t s, t;
    };

    struct EdgewalkerInput
    {
        int tile_index;
        int32_t xh, xm, xl, yh, ym, yl;
        int32_t slopeh, slopem, slopel;
        bool right_major;
        int32_t r, g, b, a;
        int32_t DrDx, DgDx, DbDx, DaDx;
        int32_t DrDy, DgDy, DbDy, DaDy;
        int32_t DrDe, DgDe, DbDe, DaDe;
        int32_t s, t, w;
        int32_t DsDx, DtDx, DwDx;
        int32_t DsDy, DtDy, DwDy;
        int32_t DsDe, DtDe, DwDe;
        int32_t z, DzDx, DzDy, DzDe;

        EdgewalkerInput()
        {
            std::memset(this, 0, sizeof(*this));
        }
    };

    struct Span
    {
        int32_t min_x, max_x;
        bool valid;
        int32_t r, g, b, a;
        int32_t s, t, w;
        int32_t z;
    };

    struct Primitive
    {
        std::array<Span, 1024> spans{};
        int32_t y_start = 0;
        int32_t y_end = 0;
        int32_t DrDx, DgDx, DbDx, DaDx;
        int32_t DsDx, DtDx, DwDx;
        int32_t DzDx;
    };

    class RDP final
    {
    public:
        RDP();
        void InstallBuses(uint8_t* rdram_ptr, uint8_t* spmem_ptr);

        void SetMIPtr(MIInterrupt* ptr)
        {
            mi_interrupt_ = ptr;
        }

        uint32_t ReadWord(uint32_t addr);
        void WriteWord(uint32_t addr, uint32_t data);
        void Reset();

        // Used for QA
        void SendCommand(const std::vector<uint64_t>& command);

    private:
        RDPStatus status_;
        uint8_t* rdram_ptr_ = nullptr;
        uint8_t* spmem_ptr_ = nullptr;
        MIInterrupt* mi_interrupt_ = nullptr;
        uint32_t start_address_;
        uint32_t end_address_;
        uint32_t current_address_;

        uint32_t zbuffer_dram_address_;

        uint32_t framebuffer_dram_address_;
        uint16_t framebuffer_width_;
        uint8_t framebuffer_format_;
        uint8_t framebuffer_pixel_size_;

        uint32_t fill_color_32_;
        uint16_t fill_color_16_0_, fill_color_16_1_;
        uint32_t blend_color_;
        uint32_t fog_color_;
        uint32_t combined_color_;
        uint32_t shade_color_;
        uint32_t primitive_color_;
        uint32_t texel_color_0_;
        uint32_t texel_color_1_;
        uint32_t environment_color_;
        uint32_t framebuffer_color_;

        uint32_t combined_alpha_;
        uint32_t texel_alpha_0_;
        uint32_t texel_alpha_1_;
        uint32_t primitive_alpha_;
        uint32_t shade_alpha_;
        uint32_t environment_alpha_;
        uint32_t fog_alpha_;

        uint32_t* color_sub_a_;
        uint32_t* color_sub_b_;
        uint32_t* color_multiplier_;
        uint32_t* color_adder_;

        uint32_t* alpha_sub_a_;
        uint32_t* alpha_sub_b_;
        uint32_t* alpha_multiplier_;
        uint32_t* alpha_adder_;

        uint8_t blender_1a_0_;
        uint8_t blender_1b_0_;
        uint8_t blender_2a_0_;
        uint8_t blender_2b_0_;

        uint32_t color_zero_ = 0;
        uint32_t color_one_ = 0xFFFF'FFFF;

        uint32_t texture_dram_address_latch_;
        uint32_t texture_width_latch_;
        uint32_t texture_pixel_size_latch_;
        Format texture_format_latch_;

        std::array<TileDescriptor, 8> tiles_;
        std::array<uint8_t, 4096> tmem_;
        std::vector<bool> rdram_9th_bit_;
        std::array<uint32_t, 0x4000> z_decompress_lut_;
        std::array<uint32_t, 0x40000> z_compress_lut_;

        bool z_update_en_ = false;
        bool z_compare_en_ = false;
        bool z_source_sel_ = false;
        bool image_read_en_ = false;
        uint8_t z_mode_ : 2 = 0;
        uint16_t primitive_depth_ = 0;
        uint16_t primitive_depth_delta_ = 0;

        uint16_t scissor_xh_ = 0;
        uint16_t scissor_yh_ = 0;
        uint16_t scissor_xl_ = 0;
        uint16_t scissor_yl_ = 0;

        enum CycleType { Cycle1, Cycle2, Copy, Fill } cycle_type_;

        void process_commands();
        void execute_command(const std::vector<uint64_t>& data);
        void draw_triangle(const std::vector<uint64_t>& data);
        inline void draw_pixel(int x, int y);
        void color_combiner();
        uint32_t blender();

        bool depth_test(int x, int y, uint32_t z, uint16_t dz);
        inline uint32_t z_get(int x, int y);
        inline uint8_t dz_get(int x, int y);
        inline void z_set(int x, int y, uint32_t z);
        inline void dz_set(int x, int y, uint8_t dz);
        uint32_t z_compress(uint32_t z);
        uint32_t z_decompress(uint32_t z);
        void init_depth_luts();
        void fetch_texels(int tile, int32_t s, int32_t t);

        uint32_t* color_get_sub_a(uint8_t sub_a);
        uint32_t* color_get_sub_b(uint8_t sub_b);
        uint32_t* color_get_mul(uint8_t mul);
        uint32_t* color_get_add(uint8_t add);
        uint32_t* alpha_get_sub_add(uint8_t sub_a);
        uint32_t* alpha_get_mul(uint8_t mul);

        EdgewalkerInput triangle_get_edgewalker_input(const std::vector<uint64_t>& data, bool shade,
                                                      bool texture, bool depth);

        template <bool Texture, bool Flip>
        EdgewalkerInput rectangle_get_edgewalker_input(const std::vector<uint64_t>& data);

        Primitive edgewalker(const EdgewalkerInput& data);

        Primitive get_angrylion_primitive(const std::vector<uint64_t>& data);
        void check_primitive(const Primitive& primitive, const std::vector<uint64_t>& data);

        friend class hydra::N64::RSP;
        friend class ::N64Debugger;
        friend class ::MmioViewer;
    };
} // namespace hydra::N64