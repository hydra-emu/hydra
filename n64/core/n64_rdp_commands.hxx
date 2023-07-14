#pragma once

namespace hydra::N64
{
    union SetColorImageCommand
    {
        uint64_t full;

        struct
        {
            uint64_t dram_address : 26;
            uint64_t              : 6;
            uint64_t width        : 10;
            uint64_t              : 9;
            uint64_t size         : 2;
            uint64_t format       : 3;
            uint64_t command      : 8 = 0x3f;
        };

        SetColorImageCommand()
        {
            full = 0;
        }
    };

    union SetFillColorCommand
    {
        uint64_t full;

        struct
        {
            uint64_t color   : 32;
            uint64_t         : 24;
            uint64_t command : 8 = 0x37;
        };

        SetFillColorCommand()
        {
            full = 0;
        }
    };

    union EdgeCoefficients
    {
        uint64_t full;

        struct
        {
            uint64_t slope : 32;
            uint64_t X     : 32;
        };

        EdgeCoefficients()
        {
            full = 0;
        }
    };

    union EdgeCoefficientsCommand
    {
        uint64_t full;

        struct
        {
            uint64_t YH      : 14;
            uint64_t         : 2;
            uint64_t YM      : 14;
            uint64_t         : 2;
            uint64_t YL      : 14;
            uint64_t         : 2;
            uint64_t Tile    : 3;
            uint64_t level   : 3;
            uint64_t         : 1;
            uint64_t lft     : 1;
            uint64_t command : 8;
        };

        EdgeCoefficientsCommand()
        {
            full = 0;
        }
    };

    union RectangleCommand
    {
        uint64_t full;

        struct
        {
            uint64_t YMIN : 12;
            uint64_t XMIN : 12;
            uint64_t Tile : 3;
            uint64_t      : 5;
            uint64_t YMAX : 12;
            uint64_t XMAX : 12;
            uint64_t      : 8;
        };

        RectangleCommand()
        {
            full = 0;
        }
    };

    union SetTileCommand
    {
        uint64_t full;

        struct
        {
            uint64_t ShiftS      : 4;
            uint64_t MaskS       : 4;
            uint64_t ms          : 1;
            uint64_t cs          : 1;
            uint64_t ShiftT      : 4;
            uint64_t MaskT       : 4;
            uint64_t mt          : 1;
            uint64_t ct          : 1;
            uint64_t Palette     : 4;
            uint64_t Tile        : 3;
            uint64_t TMemAddress : 9;
            uint64_t Line        : 9;
            uint64_t             : 1;
            uint64_t size        : 2;
            uint64_t format      : 3;
            uint64_t             : 8;
        };

        SetTileCommand()
        {
            full = 0;
        }
    };

    union LoadTileCommand
    {
        uint64_t full;

        struct
        {
            uint64_t TH   : 12;
            uint64_t SH   : 12;
            uint64_t Tile : 3;
            uint8_t       : 5;
            uint64_t TL   : 12;
            uint64_t SL   : 12;
            uint64_t      : 8;
        };

        LoadTileCommand()
        {
            full = 0;
        }
    };

    union LoadBlockCommand
    {
        uint64_t full;

        struct
        {
            uint64_t DxT  : 12;
            uint64_t SH   : 12;
            uint64_t Tile : 3;
            uint8_t       : 5;
            uint64_t TL   : 12;
            uint64_t SL   : 12;
            uint64_t      : 8;
        };

        LoadBlockCommand()
        {
            full = 0;
        }
    };

    union SetTextureImageCommand
    {
        uint64_t full;

        struct
        {
            uint64_t DRAMAddress : 26;
            uint64_t             : 6;
            uint64_t width       : 10;
            uint64_t             : 9;
            uint64_t size        : 2;
            uint64_t format      : 3;
            uint64_t             : 8;
        };

        SetTextureImageCommand()
        {
            full = 0;
        }
    };

    union SetOtherModesCommand
    {
        uint64_t full;

        struct
        {
            uint64_t                  : 2;
            uint64_t z_source_sel     : 1;
            uint64_t                  : 1;
            uint64_t z_compare_en     : 1;
            uint64_t z_update_en      : 1;
            uint64_t image_read_en    : 1;
            uint64_t color_on_cvg     : 1;
            uint64_t cvg_dest         : 2;
            uint64_t z_mode           : 2;
            uint64_t cvg_times_alpha  : 1;
            uint64_t alpha_cvg_select : 1;
            uint64_t force_blend      : 1;
            uint64_t                  : 1;
            uint64_t b_m2b_1          : 2;
            uint64_t b_m2b_0          : 2;
            uint64_t b_m2a_1          : 2;
            uint64_t b_m2a_0          : 2;
            uint64_t b_m1b_1          : 2;
            uint64_t b_m1b_0          : 2;
            uint64_t b_m1a_1          : 2;
            uint64_t b_m1a_0          : 2;
            uint64_t                  : 4;
            uint64_t                  : 16;
            uint64_t cycle_type       : 2;
            uint64_t                  : 2;
            uint64_t command          : 8 = 0x2f;
        };

        SetOtherModesCommand()
        {
            full = 0;
        }
    };

    union SetScissorCommand
    {
        uint64_t full;

        struct
        {
            uint64_t YL            : 12;
            uint64_t XL            : 12;
            uint64_t skip_even     : 1;
            uint64_t scissor_field : 1;
            uint64_t               : 6;
            uint64_t YH            : 12;
            uint64_t XH            : 12;
            uint64_t command       : 8 = 0x2d;
        };

        SetScissorCommand()
        {
            full = 0;
        }
    };

    union SetCombineModeCommand
    {
        uint64_t full;

        struct
        {
            uint64_t add_Alpha_1   : 3;
            uint64_t sub_B_Alpha_1 : 3;
            uint64_t add_RGB_1     : 3;
            uint64_t add_Alpha_0   : 3;
            uint64_t sub_B_Alpha_0 : 3;
            uint64_t add_RGB_0     : 3;
            uint64_t mul_Alpha_1   : 3;
            uint64_t sub_A_Alpha_1 : 3;
            uint64_t sub_B_RGB_1   : 4;
            uint64_t sub_B_RGB_0   : 4;
            uint64_t mul_RGB_1     : 5;
            uint64_t sub_A_RGB_1   : 4;
            uint64_t mul_Alpha_0   : 3;
            uint64_t sub_A_Alpha_0 : 3;
            uint64_t mul_RGB_0     : 5;
            uint64_t sub_A_RGB_0   : 4;
            uint64_t command       : 8 = 0x3c;
        };

        SetCombineModeCommand()
        {
            full = 0;
        }
    };
} // namespace hydra::N64