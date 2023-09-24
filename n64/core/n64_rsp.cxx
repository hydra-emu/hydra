#include <algorithm>
#include <common/compatibility.hxx>
#include <common/log.hxx>
#include <core/n64_addresses.hxx>
#include <core/n64_rdp.hxx>
#include <core/n64_rsp.hxx>
#include <fmt/format.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#define RSP_LOGGING false

bool is_sign_extension(int16_t high, int16_t low)
{
    if (high == 0)
    {
        return !(low & 0x8000);
    }
    else if (high == -1)
    {
        return (low & 0x8000);
    }
    return false;
}

int16_t clamp_signed(int64_t value)
{
    if (value < -32768)
    {
        return -32768;
    }
    if (value > 32767)
    {
        return 32767;
    }
    return value;
}

int16_t clamp_unsigned(int64_t value)
{
    if (value < 0)
    {
        return 0;
    }
    if (value >= 0x8000)
    {
        return 0xFFFF;
    }
    return value;
}

constexpr std::array<uint16_t, 0x200> RCP_TABLE = {
    0xffff, 0xff00, 0xfe01, 0xfd04, 0xfc07, 0xfb0c, 0xfa11, 0xf918, 0xf81f, 0xf727, 0xf631, 0xf53b,
    0xf446, 0xf352, 0xf25f, 0xf16d, 0xf07c, 0xef8b, 0xee9c, 0xedae, 0xecc0, 0xebd3, 0xeae8, 0xe9fd,
    0xe913, 0xe829, 0xe741, 0xe65a, 0xe573, 0xe48d, 0xe3a9, 0xe2c5, 0xe1e1, 0xe0ff, 0xe01e, 0xdf3d,
    0xde5d, 0xdd7e, 0xdca0, 0xdbc2, 0xdae6, 0xda0a, 0xd92f, 0xd854, 0xd77b, 0xd6a2, 0xd5ca, 0xd4f3,
    0xd41d, 0xd347, 0xd272, 0xd19e, 0xd0cb, 0xcff8, 0xcf26, 0xce55, 0xcd85, 0xccb5, 0xcbe6, 0xcb18,
    0xca4b, 0xc97e, 0xc8b2, 0xc7e7, 0xc71c, 0xc652, 0xc589, 0xc4c0, 0xc3f8, 0xc331, 0xc26b, 0xc1a5,
    0xc0e0, 0xc01c, 0xbf58, 0xbe95, 0xbdd2, 0xbd10, 0xbc4f, 0xbb8f, 0xbacf, 0xba10, 0xb951, 0xb894,
    0xb7d6, 0xb71a, 0xb65e, 0xb5a2, 0xb4e8, 0xb42e, 0xb374, 0xb2bb, 0xb203, 0xb14b, 0xb094, 0xafde,
    0xaf28, 0xae73, 0xadbe, 0xad0a, 0xac57, 0xaba4, 0xaaf1, 0xaa40, 0xa98e, 0xa8de, 0xa82e, 0xa77e,
    0xa6d0, 0xa621, 0xa574, 0xa4c6, 0xa41a, 0xa36e, 0xa2c2, 0xa217, 0xa16d, 0xa0c3, 0xa01a, 0x9f71,
    0x9ec8, 0x9e21, 0x9d79, 0x9cd3, 0x9c2d, 0x9b87, 0x9ae2, 0x9a3d, 0x9999, 0x98f6, 0x9852, 0x97b0,
    0x970e, 0x966c, 0x95cb, 0x952b, 0x948b, 0x93eb, 0x934c, 0x92ad, 0x920f, 0x9172, 0x90d4, 0x9038,
    0x8f9c, 0x8f00, 0x8e65, 0x8dca, 0x8d30, 0x8c96, 0x8bfc, 0x8b64, 0x8acb, 0x8a33, 0x899c, 0x8904,
    0x886e, 0x87d8, 0x8742, 0x86ad, 0x8618, 0x8583, 0x84f0, 0x845c, 0x83c9, 0x8336, 0x82a4, 0x8212,
    0x8181, 0x80f0, 0x8060, 0x7fd0, 0x7f40, 0x7eb1, 0x7e22, 0x7d93, 0x7d05, 0x7c78, 0x7beb, 0x7b5e,
    0x7ad2, 0x7a46, 0x79ba, 0x792f, 0x78a4, 0x781a, 0x7790, 0x7706, 0x767d, 0x75f5, 0x756c, 0x74e4,
    0x745d, 0x73d5, 0x734f, 0x72c8, 0x7242, 0x71bc, 0x7137, 0x70b2, 0x702e, 0x6fa9, 0x6f26, 0x6ea2,
    0x6e1f, 0x6d9c, 0x6d1a, 0x6c98, 0x6c16, 0x6b95, 0x6b14, 0x6a94, 0x6a13, 0x6993, 0x6914, 0x6895,
    0x6816, 0x6798, 0x6719, 0x669c, 0x661e, 0x65a1, 0x6524, 0x64a8, 0x642c, 0x63b0, 0x6335, 0x62ba,
    0x623f, 0x61c5, 0x614b, 0x60d1, 0x6058, 0x5fdf, 0x5f66, 0x5eed, 0x5e75, 0x5dfd, 0x5d86, 0x5d0f,
    0x5c98, 0x5c22, 0x5bab, 0x5b35, 0x5ac0, 0x5a4b, 0x59d6, 0x5961, 0x58ed, 0x5879, 0x5805, 0x5791,
    0x571e, 0x56ac, 0x5639, 0x55c7, 0x5555, 0x54e3, 0x5472, 0x5401, 0x5390, 0x5320, 0x52af, 0x5240,
    0x51d0, 0x5161, 0x50f2, 0x5083, 0x5015, 0x4fa6, 0x4f38, 0x4ecb, 0x4e5e, 0x4df1, 0x4d84, 0x4d17,
    0x4cab, 0x4c3f, 0x4bd3, 0x4b68, 0x4afd, 0x4a92, 0x4a27, 0x49bd, 0x4953, 0x48e9, 0x4880, 0x4817,
    0x47ae, 0x4745, 0x46dc, 0x4674, 0x460c, 0x45a5, 0x453d, 0x44d6, 0x446f, 0x4408, 0x43a2, 0x433c,
    0x42d6, 0x4270, 0x420b, 0x41a6, 0x4141, 0x40dc, 0x4078, 0x4014, 0x3fb0, 0x3f4c, 0x3ee8, 0x3e85,
    0x3e22, 0x3dc0, 0x3d5d, 0x3cfb, 0x3c99, 0x3c37, 0x3bd6, 0x3b74, 0x3b13, 0x3ab2, 0x3a52, 0x39f1,
    0x3991, 0x3931, 0x38d2, 0x3872, 0x3813, 0x37b4, 0x3755, 0x36f7, 0x3698, 0x363a, 0x35dc, 0x357f,
    0x3521, 0x34c4, 0x3467, 0x340a, 0x33ae, 0x3351, 0x32f5, 0x3299, 0x323e, 0x31e2, 0x3187, 0x312c,
    0x30d1, 0x3076, 0x301c, 0x2fc2, 0x2f68, 0x2f0e, 0x2eb4, 0x2e5b, 0x2e02, 0x2da9, 0x2d50, 0x2cf8,
    0x2c9f, 0x2c47, 0x2bef, 0x2b97, 0x2b40, 0x2ae8, 0x2a91, 0x2a3a, 0x29e4, 0x298d, 0x2937, 0x28e0,
    0x288b, 0x2835, 0x27df, 0x278a, 0x2735, 0x26e0, 0x268b, 0x2636, 0x25e2, 0x258d, 0x2539, 0x24e5,
    0x2492, 0x243e, 0x23eb, 0x2398, 0x2345, 0x22f2, 0x22a0, 0x224d, 0x21fb, 0x21a9, 0x2157, 0x2105,
    0x20b4, 0x2063, 0x2012, 0x1fc1, 0x1f70, 0x1f1f, 0x1ecf, 0x1e7f, 0x1e2e, 0x1ddf, 0x1d8f, 0x1d3f,
    0x1cf0, 0x1ca1, 0x1c52, 0x1c03, 0x1bb4, 0x1b66, 0x1b17, 0x1ac9, 0x1a7b, 0x1a2d, 0x19e0, 0x1992,
    0x1945, 0x18f8, 0x18ab, 0x185e, 0x1811, 0x17c4, 0x1778, 0x172c, 0x16e0, 0x1694, 0x1648, 0x15fd,
    0x15b1, 0x1566, 0x151b, 0x14d0, 0x1485, 0x143b, 0x13f0, 0x13a6, 0x135c, 0x1312, 0x12c8, 0x127f,
    0x1235, 0x11ec, 0x11a3, 0x1159, 0x1111, 0x10c8, 0x107f, 0x1037, 0x0fef, 0x0fa6, 0x0f5e, 0x0f17,
    0x0ecf, 0x0e87, 0x0e40, 0x0df9, 0x0db2, 0x0d6b, 0x0d24, 0x0cdd, 0x0c97, 0x0c50, 0x0c0a, 0x0bc4,
    0x0b7e, 0x0b38, 0x0af2, 0x0aad, 0x0a68, 0x0a22, 0x09dd, 0x0998, 0x0953, 0x090f, 0x08ca, 0x0886,
    0x0842, 0x07fd, 0x07b9, 0x0776, 0x0732, 0x06ee, 0x06ab, 0x0668, 0x0624, 0x05e1, 0x059e, 0x055c,
    0x0519, 0x04d6, 0x0494, 0x0452, 0x0410, 0x03ce, 0x038c, 0x034a, 0x0309, 0x02c7, 0x0286, 0x0245,
    0x0204, 0x01c3, 0x0182, 0x0141, 0x0101, 0x00c0, 0x0080, 0x0040,
};

constexpr std::array<uint16_t, 0x200> RSQ_TABLE = {
    0xffff, 0xff00, 0xfe02, 0xfd06, 0xfc0b, 0xfb12, 0xfa1a, 0xf923, 0xf82e, 0xf73b, 0xf648, 0xf557,
    0xf467, 0xf379, 0xf28c, 0xf1a0, 0xf0b6, 0xefcd, 0xeee5, 0xedff, 0xed19, 0xec35, 0xeb52, 0xea71,
    0xe990, 0xe8b1, 0xe7d3, 0xe6f6, 0xe61b, 0xe540, 0xe467, 0xe38e, 0xe2b7, 0xe1e1, 0xe10d, 0xe039,
    0xdf66, 0xde94, 0xddc4, 0xdcf4, 0xdc26, 0xdb59, 0xda8c, 0xd9c1, 0xd8f7, 0xd82d, 0xd765, 0xd69e,
    0xd5d7, 0xd512, 0xd44e, 0xd38a, 0xd2c8, 0xd206, 0xd146, 0xd086, 0xcfc7, 0xcf0a, 0xce4d, 0xcd91,
    0xccd6, 0xcc1b, 0xcb62, 0xcaa9, 0xc9f2, 0xc93b, 0xc885, 0xc7d0, 0xc71c, 0xc669, 0xc5b6, 0xc504,
    0xc453, 0xc3a3, 0xc2f4, 0xc245, 0xc198, 0xc0eb, 0xc03f, 0xbf93, 0xbee9, 0xbe3f, 0xbd96, 0xbced,
    0xbc46, 0xbb9f, 0xbaf8, 0xba53, 0xb9ae, 0xb90a, 0xb867, 0xb7c5, 0xb723, 0xb681, 0xb5e1, 0xb541,
    0xb4a2, 0xb404, 0xb366, 0xb2c9, 0xb22c, 0xb191, 0xb0f5, 0xb05b, 0xafc1, 0xaf28, 0xae8f, 0xadf7,
    0xad60, 0xacc9, 0xac33, 0xab9e, 0xab09, 0xaa75, 0xa9e1, 0xa94e, 0xa8bc, 0xa82a, 0xa799, 0xa708,
    0xa678, 0xa5e8, 0xa559, 0xa4cb, 0xa43d, 0xa3b0, 0xa323, 0xa297, 0xa20b, 0xa180, 0xa0f6, 0xa06c,
    0x9fe2, 0x9f59, 0x9ed1, 0x9e49, 0x9dc2, 0x9d3b, 0x9cb4, 0x9c2f, 0x9ba9, 0x9b25, 0x9aa0, 0x9a1c,
    0x9999, 0x9916, 0x9894, 0x9812, 0x9791, 0x9710, 0x968f, 0x960f, 0x9590, 0x9511, 0x9492, 0x9414,
    0x9397, 0x931a, 0x929d, 0x9221, 0x91a5, 0x9129, 0x90af, 0x9034, 0x8fba, 0x8f40, 0x8ec7, 0x8e4f,
    0x8dd6, 0x8d5e, 0x8ce7, 0x8c70, 0x8bf9, 0x8b83, 0x8b0d, 0x8a98, 0x8a23, 0x89ae, 0x893a, 0x88c6,
    0x8853, 0x87e0, 0x876d, 0x86fb, 0x8689, 0x8618, 0x85a7, 0x8536, 0x84c6, 0x8456, 0x83e7, 0x8377,
    0x8309, 0x829a, 0x822c, 0x81bf, 0x8151, 0x80e4, 0x8078, 0x800c, 0x7fa0, 0x7f34, 0x7ec9, 0x7e5e,
    0x7df4, 0x7d8a, 0x7d20, 0x7cb6, 0x7c4d, 0x7be5, 0x7b7c, 0x7b14, 0x7aac, 0x7a45, 0x79de, 0x7977,
    0x7911, 0x78ab, 0x7845, 0x77df, 0x777a, 0x7715, 0x76b1, 0x764d, 0x75e9, 0x7585, 0x7522, 0x74bf,
    0x745d, 0x73fa, 0x7398, 0x7337, 0x72d5, 0x7274, 0x7213, 0x71b3, 0x7152, 0x70f2, 0x7093, 0x7033,
    0x6fd4, 0x6f76, 0x6f17, 0x6eb9, 0x6e5b, 0x6dfd, 0x6da0, 0x6d43, 0x6ce6, 0x6c8a, 0x6c2d, 0x6bd1,
    0x6b76, 0x6b1a, 0x6abf, 0x6a64, 0x6a09, 0x6955, 0x68a1, 0x67ef, 0x673e, 0x668d, 0x65de, 0x6530,
    0x6482, 0x63d6, 0x632b, 0x6280, 0x61d7, 0x612e, 0x6087, 0x5fe0, 0x5f3a, 0x5e95, 0x5df1, 0x5d4e,
    0x5cac, 0x5c0b, 0x5b6b, 0x5acb, 0x5a2c, 0x598f, 0x58f2, 0x5855, 0x57ba, 0x5720, 0x5686, 0x55ed,
    0x5555, 0x54be, 0x5427, 0x5391, 0x52fc, 0x5268, 0x51d5, 0x5142, 0x50b0, 0x501f, 0x4f8e, 0x4efe,
    0x4e6f, 0x4de1, 0x4d53, 0x4cc6, 0x4c3a, 0x4baf, 0x4b24, 0x4a9a, 0x4a10, 0x4987, 0x48ff, 0x4878,
    0x47f1, 0x476b, 0x46e5, 0x4660, 0x45dc, 0x4558, 0x44d5, 0x4453, 0x43d1, 0x434f, 0x42cf, 0x424f,
    0x41cf, 0x4151, 0x40d2, 0x4055, 0x3fd8, 0x3f5b, 0x3edf, 0x3e64, 0x3de9, 0x3d6e, 0x3cf5, 0x3c7c,
    0x3c03, 0x3b8b, 0x3b13, 0x3a9c, 0x3a26, 0x39b0, 0x393a, 0x38c5, 0x3851, 0x37dd, 0x3769, 0x36f6,
    0x3684, 0x3612, 0x35a0, 0x352f, 0x34bf, 0x344f, 0x33df, 0x3370, 0x3302, 0x3293, 0x3226, 0x31b9,
    0x314c, 0x30df, 0x3074, 0x3008, 0x2f9d, 0x2f33, 0x2ec8, 0x2e5f, 0x2df6, 0x2d8d, 0x2d24, 0x2cbc,
    0x2c55, 0x2bee, 0x2b87, 0x2b21, 0x2abb, 0x2a55, 0x29f0, 0x298b, 0x2927, 0x28c3, 0x2860, 0x27fd,
    0x279a, 0x2738, 0x26d6, 0x2674, 0x2613, 0x25b2, 0x2552, 0x24f2, 0x2492, 0x2432, 0x23d3, 0x2375,
    0x2317, 0x22b9, 0x225b, 0x21fe, 0x21a1, 0x2145, 0x20e8, 0x208d, 0x2031, 0x1fd6, 0x1f7b, 0x1f21,
    0x1ec7, 0x1e6d, 0x1e13, 0x1dba, 0x1d61, 0x1d09, 0x1cb1, 0x1c59, 0x1c01, 0x1baa, 0x1b53, 0x1afc,
    0x1aa6, 0x1a50, 0x19fa, 0x19a5, 0x1950, 0x18fb, 0x18a7, 0x1853, 0x17ff, 0x17ab, 0x1758, 0x1705,
    0x16b2, 0x1660, 0x160d, 0x15bc, 0x156a, 0x1519, 0x14c8, 0x1477, 0x1426, 0x13d6, 0x1386, 0x1337,
    0x12e7, 0x1298, 0x1249, 0x11fb, 0x11ac, 0x115e, 0x1111, 0x10c3, 0x1076, 0x1029, 0x0fdc, 0x0f8f,
    0x0f43, 0x0ef7, 0x0eab, 0x0e60, 0x0e15, 0x0dca, 0x0d7f, 0x0d34, 0x0cea, 0x0ca0, 0x0c56, 0x0c0c,
    0x0bc3, 0x0b7a, 0x0b31, 0x0ae8, 0x0aa0, 0x0a58, 0x0a10, 0x09c8, 0x0981, 0x0939, 0x08f2, 0x08ab,
    0x0865, 0x081e, 0x07d8, 0x0792, 0x074d, 0x0707, 0x06c2, 0x067d, 0x0638, 0x05f3, 0x05af, 0x056a,
    0x0526, 0x04e2, 0x049f, 0x045b, 0x0418, 0x03d5, 0x0392, 0x0350, 0x030d, 0x02cb, 0x0289, 0x0247,
    0x0206, 0x01c4, 0x0183, 0x0142, 0x0101, 0x00c0, 0x0080, 0x0040,
};

namespace hydra::N64
{
#define vuinstr (VUInstruction(instruction_.full))
#define rdreg (gpr_regs_[instruction_.RType.rd])
#define rsreg (gpr_regs_[instruction_.RType.rs])
#define rtreg (gpr_regs_[instruction_.RType.rt])
#define saval (instruction_.RType.sa)
#define immval (instruction_.IType.immediate)
#define seimmval (static_cast<int64_t>(static_cast<int16_t>(instruction_.IType.immediate)))

    template <>
    void RSP::log_cpu_state<false>(bool, uint64_t)
    {
    }

    template <>
    void RSP::log_cpu_state<true>(bool use_crc, uint64_t instructions)
    {
        static uint64_t count = 0;
        count++;
        if (count >= instructions)
        {
            exit(1);
        }
        // Get crc32 of gpr and fpr regs
        if (use_crc)
        {
            uint32_t gprcrc = 0xFFFF'FFFF;
            uint32_t veccrc = 0xFFFF'FFFF;
            // uint32_t memcrc = 0xFFFF'FFFF;
            for (int i = 0; i < 32; i++)
            {
                gprcrc = hydra::crc32_u64(gprcrc, gpr_regs_[i].UW);
                for (int j = 0; j < 8; j++)
                {
                    veccrc = hydra::crc32_u16(veccrc, vu_regs_[i][j]);
                }
            }
            // for (int i = 0; i < 0x1000; i += 4)
            // {
            //     memcrc = hydra::crc32_u32(memcrc, hydra::bswap32(load_word(i)));
            // }
            // memcrc ^= 0xFFFF'FFFF;
            gprcrc ^= 0xFFFF'FFFF;
            veccrc ^= 0xFFFF'FFFF;
            printf("RSP: %08x %08x %08x", pc_, instruction_.full, gprcrc);
        }
        else
        {
            printf("%08x %08x", pc_, instruction_.full);
            for (int i = 1; i < 32; i++)
            {
                printf(" %08x", gpr_regs_[i].UW);
            }
        }
        printf("\n");
    }

    RSP::RSP()
    {
        status_.halt = true;
    }

    void RSP::Reset()
    {
        status_.full = 0;
        status_.halt = true;
        mem_.fill(0);
        std::for_each(gpr_regs_.begin(), gpr_regs_.end(), [](auto& reg) { reg.UW = 0; });
        std::for_each(vu_regs_.begin(), vu_regs_.end(), [](auto& reg) { reg.fill(0); });
        std::for_each(accumulator_.begin(), accumulator_.end(), [](auto& reg) { reg.Set(0); });
        vco_.Clear();
        vce_.Clear();
        vcc_.Clear();
        div_in_ = 0;
        div_out_ = 0;
        div_in_ready_ = false;
        instruction_.full = 0;
        mem_addr_ = 0;
        dma_imem_ = false;
        rdram_addr_ = 0;
        rd_len_ = 0;
        wr_len_ = 0;
        pc_ = 0;
        next_pc_ = 4;
        semaphore_ = false;
    }

    void RSP::Tick()
    {
        gpr_regs_[0].UW = 0;
        auto instruction = fetch_instruction();
        instruction_.full = instruction;

        log_cpu_state<RSP_LOGGING>(true, 10000000);

        pc_ = next_pc_ & 0xFFF;
        next_pc_ = (pc_ + 4) & 0xFFF;
        execute_instruction();
    }

    void RSP::execute_instruction()
    {
        (instruction_table_[instruction_.IType.op])(this);
    }

    uint32_t RSP::fetch_instruction()
    {
        uint32_t instruction = *reinterpret_cast<uint32_t*>(&mem_[0x1000 + (pc_ & 0xFFF)]);
        return hydra::bswap32(instruction);
    }

    uint8_t RSP::load_byte(uint16_t address)
    {
        return mem_[address & 0xFFF];
    }

    uint16_t RSP::load_halfword(uint16_t address)
    {
        uint16_t data = mem_[address & 0xFFF] | (mem_[(address + 1) & 0xFFF] << 8);
        return hydra::bswap16(data);
    }

    uint32_t RSP::load_word(uint16_t address)
    {
        uint32_t data = mem_[address & 0xFFF] | (mem_[(address + 1) & 0xFFF] << 8) |
                        (mem_[(address + 2) & 0xFFF] << 16) | (mem_[(address + 3) & 0xFFF] << 24);
        return hydra::bswap32(data);
    }

    void RSP::store_byte(uint16_t address, uint8_t data)
    {
        mem_[address & 0xFFF] = data;
    }

    void RSP::store_halfword(uint16_t address, uint16_t data)
    {
        data = hydra::bswap16(data);
        mem_[address & 0xFFF] = data & 0xFF;
        mem_[(address + 1) & 0xFFF] = (data >> 8) & 0xFF;
    }

    void RSP::store_word(uint16_t address, uint32_t data)
    {
        data = hydra::bswap32(data);
        mem_[address & 0xFFF] = data & 0xFF;
        mem_[(address + 1) & 0xFFF] = (data >> 8) & 0xFF;
        mem_[(address + 2) & 0xFFF] = (data >> 16) & 0xFF;
        mem_[(address + 3) & 0xFFF] = (data >> 24) & 0xFF;
    }

    void RSP::branch_to(uint16_t address)
    {
        // TODO: Make it so that addresses are always correct (i.e. don't & on EVERY tick)
        next_pc_ = (address & ~0b11) & 0xFFF;
    }

    void RSP::conditional_branch(bool condition, uint16_t address)
    {
        if (condition)
        {
            branch_to(address);
        }
    }

    void RSP::link_register(uint8_t reg)
    {
        gpr_regs_[reg].UW = pc_ + 4;
    }

    void RSP::read_dma()
    {
        auto bytes_per_row = (rd_len_ & 0xFFF) + 1;
        bytes_per_row = (bytes_per_row + 0x7) & ~0x7;
        uint32_t row_count = (rd_len_ >> 12) & 0xFF;
        uint32_t row_stride = (rd_len_ >> 20) & 0xFFF;
        auto rdram_index = rdram_addr_ & 0xFFFFF8;
        auto rsp_index = mem_addr_ & 0xFF8;
        uint8_t* dest = dma_imem_ ? &mem_[0x1000] : &mem_[0];
        uint8_t* source = rdram_ptr_;

        for (uint32_t i = 0; i < row_count + 1; i++)
        {
            for (uint32_t j = 0; j < bytes_per_row; j++)
            {
                dest[rsp_index++] = source[rdram_index++];
            }
            rdram_index += row_stride;
            rdram_index &= 0xFFFFF8;
            rsp_index &= 0xFF8;
        }

        mem_addr_ = rsp_index;
        mem_addr_ |= dma_imem_ ? 0x1000 : 0;
        rdram_addr_ = rdram_index;
        // After the DMA transfer is finished, this field contains the value 0xFF8
        // The reason is that the field is internally decremented by 8 for each transferred word
        // so the final value will be -8 (in hex, 0xFF8)
        rd_len_ = (row_stride << 20) | 0xFF8;

        if constexpr (RSP_LOGGING)
        {
            // printf("RSP: copied %d bytes\n", copy);
            // dump_mem();
        }
    }

    void RSP::write_dma()
    {
        auto bytes_per_row = (wr_len_ & 0xFFF) + 1;
        bytes_per_row = (bytes_per_row + 0x7) & ~0x7;
        uint32_t row_count = (wr_len_ >> 12) & 0xFF;
        uint32_t row_stride = (wr_len_ >> 20) & 0xFFF;
        auto rdram_index = rdram_addr_ & 0xFFFFF8;
        auto rsp_index = mem_addr_ & 0xFF8;
        uint8_t* dest = rdram_ptr_;
        uint8_t* source = dma_imem_ ? &mem_[0x1000] : &mem_[0];

        for (uint32_t i = 0; i < row_count + 1; i++)
        {
            for (uint32_t j = 0; j < bytes_per_row; j++)
            {
                dest[rdram_index++] = source[rsp_index++];
            }
            rdram_index += row_stride;
            rdram_index &= 0xFFFFF8;
            rsp_index &= 0xFF8;
        }

        mem_addr_ = rsp_index;
        mem_addr_ |= dma_imem_ ? 0x1000 : 0;
        rdram_addr_ = rdram_index;
        // After the DMA transfer is finished, this field contains the value 0xFF8
        // The reason is that the field is internally decremented by 8 for each transferred word
        // so the final value will be -8 (in hex, 0xFF8)
        wr_len_ = (row_stride << 20) | 0xFF8;
    }

    void RSP::dump_mem()
    {
        printf("rsp dma:\n");
        for (int i = 0; i < 0x2000; i += 4)
        {
            printf("%d: %02x %02x %02x %02x\n", i, mem_[i + 3], mem_[i + 2], mem_[i + 1],
                   mem_[i + 0]);
        }
        printf("\n");
    }

    void RSP::write_hwio(RSPHWIO addr, uint32_t data)
    {
        switch (addr)
        {
            case RSPHWIO::Cache:
            {
                mem_addr_ = data & 0b1111'1111'1111;
                dma_imem_ = data & 0b1'0000'0000'0000;
                break;
            }
            case RSPHWIO::DramAddr:
            {
                rdram_addr_ = data & 0b111'1111'1111'1111'1111'1111;
                break;
            }
            case RSPHWIO::RdLen:
            {
                rd_len_ = data;
                read_dma();
                break;
            }
            case RSPHWIO::WrLen:
            {
                wr_len_ = data;
                write_dma();
                break;
            }
            case RSPHWIO::Full:
            {
                status_.dma_full = data;
                break;
            }
            case RSPHWIO::Busy:
            {
                status_.dma_busy = data;
                break;
            }
            case RSPHWIO::Semaphore:
            {
                semaphore_ = false;
                break;
            }
            case RSPHWIO::Status:
            {
                RSPStatusWrite sp_write;
                sp_write.full = data;
                if (sp_write.clear_intr && !sp_write.set_intr)
                {
                    interrupt_callback_(false);
                }
                else if (!sp_write.clear_intr && sp_write.set_intr)
                {
                    Logger::Debug("Raising SP interrupt");
                    interrupt_callback_(true);
                }
                if (sp_write.clear_broke)
                {
                    status_.broke = false;
                }
#define flag(x)                                  \
    if (!sp_write.set_##x && sp_write.clear_##x) \
    {                                            \
        status_.x = false;                       \
    }                                            \
    if (sp_write.set_##x && !sp_write.clear_##x) \
    {                                            \
        status_.x = true;                        \
    }
                flag(signal_0);
                flag(signal_1);
                flag(signal_2);
                flag(signal_3);
                flag(signal_4);
                flag(signal_5);
                flag(signal_6);
                flag(signal_7);
                flag(halt);
                flag(intr_break);
                flag(sstep);
#undef flag
                break;
            }
            case RSPHWIO::CmdStart:
            {
                rdp_ptr_->WriteWord(DP_START, data);
                break;
            }
            case RSPHWIO::CmdEnd:
            {
                rdp_ptr_->WriteWord(DP_END, data);
                break;
            }
            case RSPHWIO::CmdStatus:
            {
                rdp_ptr_->WriteWord(DP_STATUS, data);
                break;
            }
            default:
            {
                Logger::Fatal("Unimplemented RSP HWIO write: {}", static_cast<int>(addr));
                break;
            }
        }
    }

    uint32_t RSP::read_hwio(RSPHWIO addr)
    {
        switch (addr)
        {
            case RSPHWIO::Cache:
                return mem_addr_;
            case RSPHWIO::DramAddr:
                return rdram_addr_;
            case RSPHWIO::RdLen:
                return rd_len_;
            case RSPHWIO::WrLen:
                return wr_len_;
            case RSPHWIO::Status:
                return status_.full;
            case RSPHWIO::Full:
                return status_.dma_full;
            case RSPHWIO::Busy:
                return status_.dma_busy;
            case RSPHWIO::Semaphore:
            {
                bool value = semaphore_;
                semaphore_ = true;
                return value;
            }
            case RSPHWIO::CmdStart:
            {
                return rdp_ptr_->start_address_;
            }
            case RSPHWIO::CmdEnd:
            {
                return rdp_ptr_->end_address_;
            }
            case RSPHWIO::CmdCurrent:
            {
                return rdp_ptr_->current_address_;
            }
            case RSPHWIO::CmdStatus:
            {
                return rdp_ptr_->ReadWord(DP_STATUS);
            }
            case RSPHWIO::CmdClock:
            {
                Logger::Warn("Unimplemented RDP command clock read");
                return 0;
            }
            case RSPHWIO::CmdBusy:
            {
                Logger::Warn("Unimplemented RDP command busy read");
                return 0;
            }
            case RSPHWIO::CmdPipeBusy:
            {
                Logger::Warn("Unimplemented RDP command pipe busy read");
                return 0;
            }
            case RSPHWIO::CmdTmemBusy:
            {
                Logger::Warn("Unimplemented RDP command TMEM busy read");
                return 0;
            }
            default:
            {
                Logger::Fatal("Illegal RSP HWIO read: {}", static_cast<int>(addr));
                return 0;
            }
        }
    }

    bool RSP::IsHalted()
    {
        return status_.halt;
    }

    void RSP::InstallBuses(uint8_t* rdram_ptr, RDP* rdp_ptr)
    {
        rdram_ptr_ = rdram_ptr;
        rdp_ptr_ = rdp_ptr;
    }

    void RSP::SetInterruptCallback(std::function<void(bool)> callback)
    {
        interrupt_callback_ = callback;
    }

    using Elements = std::array<uint8_t, 8>;

    std::array<Elements, 16> elements = {{{0, 1, 2, 3, 4, 5, 6, 7},
                                          {0, 1, 2, 3, 4, 5, 6, 7},
                                          {0, 0, 2, 2, 4, 4, 6, 6},
                                          {1, 1, 3, 3, 5, 5, 7, 7},
                                          {0, 0, 0, 0, 4, 4, 4, 4},
                                          {1, 1, 1, 1, 5, 5, 5, 5},
                                          {2, 2, 2, 2, 6, 6, 6, 6},
                                          {3, 3, 3, 3, 7, 7, 7, 7},
                                          {0, 0, 0, 0, 0, 0, 0, 0},
                                          {1, 1, 1, 1, 1, 1, 1, 1},
                                          {2, 2, 2, 2, 2, 2, 2, 2},
                                          {3, 3, 3, 3, 3, 3, 3, 3},
                                          {4, 4, 4, 4, 4, 4, 4, 4},
                                          {5, 5, 5, 5, 5, 5, 5, 5},
                                          {6, 6, 6, 6, 6, 6, 6, 6},
                                          {7, 7, 7, 7, 7, 7, 7, 7}}};

    VectorRegister& RSP::get_vt()
    {
        return vu_regs_[vuinstr.vt];
    }

    VectorRegister& RSP::get_vs()
    {
        return vu_regs_[vuinstr.vs];
    }

    VectorRegister& RSP::get_vd()
    {
        return vu_regs_[vuinstr.vd];
    }

    int16_t RSP::get_lane(int reg, int lane)
    {
        lane &= 15;
        if (lane == 15)
        {
            return vu_regs_[reg][7] << 8 | vu_regs_[reg][0] >> 8;
        }
        else if (lane & 1)
        {
            return (vu_regs_[reg][lane >> 1] << 8) | (vu_regs_[reg][(lane >> 1) + 1] >> 8);
        }
        else
        {
            return vu_regs_[reg][lane >> 1];
        }
    }

    void RSP::set_lane(int reg, int lane, int16_t value)
    {
        if (lane > 15)
        {
            return;
        }
        if (lane == 15)
        {
            vu_regs_[reg][7] = (vu_regs_[reg][7] & ~0xFF) | ((value >> 8) & 0xFF);
        }
        else if (lane & 1)
        {
            vu_regs_[reg][lane >> 1] = (vu_regs_[reg][lane >> 1] & ~0xFF) | ((value >> 8) & 0xFF);
            vu_regs_[reg][(lane >> 1) + 1] =
                (vu_regs_[reg][(lane >> 1) + 1] & 0xFF) | ((value << 8) & ~0xFF);
        }
        else
        {
            vu_regs_[reg][lane >> 1] = value;
        }
    }

    int16_t RSP::get_control(int reg)
    {
        switch (reg & 0b11)
        {
            case 0:
                return *vco_;
            case 1:
                return *vcc_;
            case 2:
            case 3:
                return *vce_;
        }
        return 0;
    }

    void RSP::set_control(int reg, int16_t value)
    {
        switch (reg & 0b11)
        {
            case 0:
                *vco_ = value;
                break;
            case 1:
                *vcc_ = value;
                break;
            case 2:
            case 3:
                *vce_ = value;
                break;
        }
    }

    void RSP::ERROR2()
    {
        Logger::Warn("Ran ERROR2 instruction: {:#x}", instruction_.full);
    }

    void RSP::LWC2()
    {
        switch (instruction_.WCType.opcode)
        {
            case 0x0:
                return LBV();
            case 0x1:
                return LSV();
            case 0x2:
                return LLV();
            case 0x3:
                return LDV();
            case 0x4:
                return LQV();
            case 0x5:
                return LRV();
            case 0x6:
                return LPV();
            case 0x7:
                return LUV();
            case 0xB:
                return LTV();
            default:
            {
                Logger::Warn("LWC2: {:08x}", static_cast<uint8_t>(instruction_.WCType.opcode));
            }
        }
    }

    void RSP::SWC2()
    {
        switch (instruction_.WCType.opcode)
        {
            case 0x0:
                return SBV();
            case 0x1:
                return SSV();
            case 0x2:
                return SLV();
            case 0x3:
                return SDV();
            case 0x4:
                return SQV();
            case 0x5:
                return SRV();
            case 0x6:
                return SPV();
            case 0x7:
                return SUV();
            case 0xB:
                return STV();
            default:
            {
                Logger::Warn("SWC2: {:08x}", static_cast<uint8_t>(instruction_.WCType.opcode));
            }
        }
    }

    void RSP::SBV()
    {
        int reg = instruction_.WCType.vt;
        int lane = instruction_.WCType.element;
        uint32_t address = gpr_regs_[instruction_.WCType.base].UW +
                           (static_cast<int8_t>(instruction_.WCType.offset << 1) >> 1);
        store_byte(address, get_lane(reg, lane) >> 8);
    }

    void RSP::SSV()
    {
        int reg = instruction_.WCType.vt;
        int lane = instruction_.WCType.element;
        uint32_t address = gpr_regs_[instruction_.WCType.base].UW +
                           static_cast<int8_t>(instruction_.WCType.offset << 1);
        store_halfword(address, get_lane(reg, lane));
    }

    void RSP::SLV()
    {
        int reg = instruction_.WCType.vt;
        int lane = instruction_.WCType.element;
        uint32_t address = gpr_regs_[instruction_.WCType.base].UW +
                           (static_cast<int8_t>(instruction_.WCType.offset << 1) << 1);
        for (uint32_t i = 0; i < 4; i += 2)
        {
            store_halfword(address + i, get_lane(reg, lane + i));
        }
    }

    void RSP::SDV()
    {
        int reg = instruction_.WCType.vt;
        int lane = instruction_.WCType.element;
        uint32_t address = gpr_regs_[instruction_.WCType.base].UW +
                           (static_cast<int8_t>(instruction_.WCType.offset << 1) << 2);
        for (uint32_t i = 0; i < 8; i += 2)
        {
            store_halfword(address + i, get_lane(reg, lane + i));
        }
    }

    void RSP::SQV()
    {
        int reg = instruction_.WCType.vt;
        int lane = instruction_.WCType.element;
        uint32_t address = gpr_regs_[instruction_.WCType.base].UW +
                           (static_cast<int8_t>(instruction_.WCType.offset << 1) << 3);

        for (uint32_t i = 0; i < 15 - (address & 0xF); i += 2)
        {
            store_halfword(address + i, get_lane(reg, lane + i));
        }

        if (address & 0x1)
        {
            int i = 15 - (address & 0xF);
            address = address + i;
            store_byte(address, get_lane(reg, lane + i) >> 8);
        }
    }

    void RSP::SRV()
    {
        int reg = instruction_.WCType.vt;
        int lane = instruction_.WCType.element;
        uint32_t address = gpr_regs_[instruction_.WCType.base].UW +
                           (static_cast<int8_t>(instruction_.WCType.offset << 1) << 3);

        for (int i = 16 - (address & 0xF); i < 15; i += 2)
        {
            store_halfword(address + i - 16, get_lane(reg, lane + i));
        }

        if (address & 0x1)
        {
            int i = 15;
            address = address + i - 16;
            store_byte(address, get_lane(reg, lane + i) >> 8);
        }
    }

    void RSP::SPV()
    {
        int reg = instruction_.WCType.vt;
        int lane = instruction_.WCType.element;
        uint32_t address = gpr_regs_[instruction_.WCType.base].UW +
                           (static_cast<int8_t>(instruction_.WCType.offset << 1) << 2);

        for (int i = 0; i < 8; i++)
        {
            if (((lane + i) & 15) < 8)
            {
                store_byte(address + i, static_cast<uint16_t>(get_lane(reg, (lane + i) << 1)) >> 8);
            }
            else
            {
                store_byte(address + i, static_cast<uint16_t>(get_lane(reg, (lane + i) << 1)) >> 7);
            }
        }
    }

    void RSP::SUV()
    {
        int reg = instruction_.WCType.vt;
        int lane = instruction_.WCType.element;
        uint32_t address = gpr_regs_[instruction_.WCType.base].UW +
                           (static_cast<int8_t>(instruction_.WCType.offset << 1) << 2);

        for (int i = 0; i < 8; i++)
        {
            if (((lane + i) & 15) < 8)
            {
                store_byte(address + i, static_cast<uint16_t>(get_lane(reg, (lane + i) << 1)) >> 7);
            }
            else
            {
                store_byte(address + i, static_cast<uint16_t>(get_lane(reg, (lane + i) << 1)) >> 8);
            }
        }
    }

    void RSP::STV()
    {
        int reg = instruction_.WCType.vt;
        int lane = instruction_.WCType.element;
        uint32_t address = gpr_regs_[instruction_.WCType.base].UW +
                           (static_cast<int8_t>(instruction_.WCType.offset << 1) << 3);

        for (int i = 0; i < 16; i += 2)
        {
            uint16_t a = (address & 0xFF0) + ((address + i) & 0xF);
            uint16_t b = ((lane + i) & 0xF) / 2;
            store_byte(a, get_lane(reg, b));
        }
    }

    void RSP::LBV()
    {
        int reg = instruction_.WCType.vt;
        int lane = instruction_.WCType.element;
        uint32_t address = gpr_regs_[instruction_.WCType.base].UW +
                           (static_cast<int8_t>(instruction_.WCType.offset << 1) >> 1);
        set_lane(reg, lane, (get_lane(reg, lane) & 0xFF) | (load_byte(address) << 8));
    }

    void RSP::LSV()
    {
        int reg = instruction_.WCType.vt;
        int lane = instruction_.WCType.element;
        uint32_t address = gpr_regs_[instruction_.WCType.base].UW +
                           static_cast<int8_t>(instruction_.WCType.offset << 1);
        set_lane(reg, lane, load_halfword(address));
    }

    void RSP::LLV()
    {
        int reg = instruction_.WCType.vt;
        int lane = instruction_.WCType.element;
        uint32_t address = gpr_regs_[instruction_.WCType.base].UW +
                           (static_cast<int8_t>(instruction_.WCType.offset << 1) << 1);
        for (int i = 0; i < 4; i += 2)
        {
            set_lane(reg, lane + i, load_halfword(address + i));
        }
    }

    void RSP::LDV()
    {
        int reg = instruction_.WCType.vt;
        int lane = instruction_.WCType.element;
        uint32_t address = gpr_regs_[instruction_.WCType.base].UW +
                           (static_cast<int8_t>(instruction_.WCType.offset << 1) << 2);
        for (uint32_t i = 0; i < 8; i += 2)
        {
            set_lane(reg, lane + i, load_halfword(address + i));
        }
    }

    void RSP::LQV()
    {
        int reg = instruction_.WCType.vt;
        int lane = instruction_.WCType.element;
        uint32_t address = gpr_regs_[instruction_.WCType.base].UW +
                           (static_cast<int8_t>(instruction_.WCType.offset << 1) << 3);

        for (uint32_t i = 0; i < 15 - (address & 0xF); i += 2)
        {
            set_lane(reg, lane + i, load_halfword(address + i));
        }

        if (address & 0x1)
        {
            int i = 15 - (address & 0xF);
            uint8_t value = load_byte(address + i);
            uint16_t old_value = get_lane(reg, lane + i);
            set_lane(reg, lane + i, (old_value & 0xFF) | (value << 8));
        }
    }

    void RSP::LRV()
    {
        int reg = instruction_.WCType.vt;
        int lane = instruction_.WCType.element;
        uint32_t address = gpr_regs_[instruction_.WCType.base].UW +
                           (static_cast<int8_t>(instruction_.WCType.offset << 1) << 3);

        for (int i = 16 - (address & 0xF); i < 16; i += 2)
        {
            set_lane(reg, lane + i, load_halfword(address + i - 16));
        }
    }

    void RSP::LPV()
    {
        int reg = instruction_.WCType.vt;
        int lane = instruction_.WCType.element;
        uint32_t address = gpr_regs_[instruction_.WCType.base].UW +
                           (static_cast<int8_t>(instruction_.WCType.offset << 1) << 2);

        for (int i = 0; i < 8; i++)
        {
            set_lane(reg, lane + i * 2, load_byte(address + i) << 8);
        }
    }

    void RSP::LUV()
    {
        int reg = instruction_.WCType.vt;
        int lane = instruction_.WCType.element;
        uint32_t address = gpr_regs_[instruction_.WCType.base].UW +
                           (static_cast<int8_t>(instruction_.WCType.offset << 1) << 2);

        for (int i = 0; i < 8; i++)
        {
            set_lane(reg, lane + i * 2, load_byte(address + i) << 7);
        }
    }

    void RSP::LTV()
    {
        int reg = instruction_.WCType.vt;
        int lane = instruction_.WCType.element;
        uint32_t address = gpr_regs_[instruction_.WCType.base].UW +
                           (static_cast<int8_t>(instruction_.WCType.offset << 1) << 3);

        for (int i = 0; i < 16; i += 2)
        {
            uint8_t b = (lane + i) & 0xF;
            set_lane(reg + b / 2, i, load_halfword(address + b) & 0xFFF);
        }
    }

    void RSP::VAND()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vs = get_vs();
        VectorRegister& vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        for (int i = 0; i < 8; i++)
        {
            accumulator_[i].SetLow(vs[i] & vt[e[i]]);
        }

        for (int i = 0; i < 8; i++)
        {
            vd[i] = accumulator_[i].GetLow();
        }
    }

    void RSP::VNAND()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vs = get_vs();
        VectorRegister& vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        for (int i = 0; i < 8; i++)
        {
            accumulator_[i].SetLow((vs[i] & vt[e[i]]) ^ 0xFFFF);
        }

        for (int i = 0; i < 8; i++)
        {
            vd[i] = accumulator_[i].GetLow();
        }
    }

    void RSP::VOR()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vs = get_vs();
        VectorRegister& vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        for (int i = 0; i < 8; i++)
        {
            accumulator_[i].SetLow(vs[i] | vt[e[i]]);
        }

        for (int i = 0; i < 8; i++)
        {
            vd[i] = accumulator_[i].GetLow();
        }
    }

    void RSP::VNOR()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vs = get_vs();
        VectorRegister& vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        for (int i = 0; i < 8; i++)
        {
            accumulator_[i].SetLow((vs[i] | vt[e[i]]) ^ 0xFFFF);
        }
        for (int i = 0; i < 8; i++)
        {
            vd[i] = accumulator_[i].GetLow();
        }
    }

    void RSP::VXOR()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vs = get_vs();
        VectorRegister& vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        for (int i = 0; i < 8; i++)
        {
            accumulator_[i].SetLow(vs[i] ^ vt[e[i]]);
        }

        for (int i = 0; i < 8; i++)
        {
            vd[i] = accumulator_[i].GetLow();
        }
    }

    void RSP::VNXOR()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vs = get_vs();
        VectorRegister& vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        for (int i = 0; i < 8; i++)
        {
            accumulator_[i].SetLow((vs[i] ^ vt[e[i]]) ^ 0xFFFF);
        }

        for (int i = 0; i < 8; i++)
        {
            vd[i] = accumulator_[i].GetLow();
        }
    }

    void RSP::VSAR()
    {
        VectorRegister& vd = get_vd();
        switch (vuinstr.element)
        {
            case 0x8:
            {
                for (int i = 0; i < 8; i++)
                {
                    vd[i] = accumulator_[i].GetHigh();
                }
                break;
            }
            case 0x9:
            {
                for (int i = 0; i < 8; i++)
                {
                    vd[i] = accumulator_[i].GetMiddle();
                }
                break;
            }
            case 0xA:
            {
                for (int i = 0; i < 8; i++)
                {
                    vd[i] = accumulator_[i].GetLow();
                }
                break;
            }
            default:
            {
                for (int i = 0; i < 8; i++)
                {
                    vd[i] = 0;
                }
                break;
            }
        }
    }

    void RSP::VMULF()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vs = get_vs();
        VectorRegister& vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        for (int i = 0; i < 8; i++)
        {
            int64_t product = static_cast<int16_t>(vs[i]) * static_cast<int16_t>(vt[e[i]]);
            accumulator_[i].Set((product << 1) + 0x8000);
        }

        for (int i = 0; i < 8; i++)
        {
            vd[i] = clamp_signed(accumulator_[i].GetSigned() >> 16);
        }
    }

    void RSP::VMULU()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vs = get_vs();
        VectorRegister& vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        for (int i = 0; i < 8; i++)
        {
            int64_t product = static_cast<int16_t>(vs[i]) * static_cast<int16_t>(vt[e[i]]);
            accumulator_[i].Set((product << 1) + 0x8000);
        }

        for (int i = 0; i < 8; i++)
        {
            vd[i] = clamp_unsigned(accumulator_[i].GetSigned() >> 16);
        }
    }

    void RSP::VMUDL()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vs = get_vs();
        VectorRegister vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        for (int i = 0; i < 8; i++)
        {
            uint64_t product = static_cast<uint64_t>(vs[i]) * vt[e[i]];
            accumulator_[i].Set(product >> 16);
        }

        for (int i = 0; i < 8; i++)
        {
            if (is_sign_extension(accumulator_[i].GetHigh(), accumulator_[i].GetMiddle()))
            {
                vd[i] = accumulator_[i].GetLow();
            }
            else if (accumulator_[i].GetHighSigned() < 0)
            {
                vd[i] = 0;
            }
            else
            {
                vd[i] = 0xFFFF;
            }
        }
    }

    void RSP::VMUDM()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vs = get_vs();
        VectorRegister& vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        for (int i = 0; i < 8; i++)
        {
            int64_t product = vt[e[i]] * static_cast<int16_t>(vs[i]);
            accumulator_[i].Set(product);
        }

        for (int i = 0; i < 8; i++)
        {
            vd[i] = clamp_signed(accumulator_[i].GetSigned() >> 16);
        }
    }

    void RSP::VMUDN()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vs = get_vs();
        VectorRegister& vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        for (int i = 0; i < 8; i++)
        {
            int64_t product = vs[i] * static_cast<int16_t>(vt[e[i]]);
            accumulator_[i].Set(product);
        }

        for (int i = 0; i < 8; i++)
        {
            vd[i] = accumulator_[i].Get();
        }
    }

    void RSP::VMUDH()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vs = get_vs();
        VectorRegister& vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        for (int i = 0; i < 8; i++)
        {
            int64_t product = static_cast<int16_t>(vs[i]) * static_cast<int16_t>(vt[e[i]]);
            accumulator_[i].Set(product << 16);
        }

        for (int i = 0; i < 8; i++)
        {
            vd[i] = clamp_signed(accumulator_[i].GetSigned() >> 16);
        }
    }

    void RSP::VMACF()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vs = get_vs();
        VectorRegister& vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        for (int i = 0; i < 8; i++)
        {
            int64_t product = static_cast<int16_t>(vs[i]) * static_cast<int16_t>(vt[e[i]]);
            accumulator_[i].Add(product << 1);
        }

        for (int i = 0; i < 8; i++)
        {
            vd[i] = clamp_signed(accumulator_[i].GetSigned() >> 16);
        }
    }

    void RSP::VMACU()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vs = get_vs();
        VectorRegister& vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        for (int i = 0; i < 8; i++)
        {
            int64_t product = static_cast<int16_t>(vs[i]) * static_cast<int16_t>(vt[e[i]]);
            accumulator_[i].Add(product << 1);
        }

        for (int i = 0; i < 8; i++)
        {
            vd[i] = clamp_unsigned(accumulator_[i].GetSigned() >> 16);
        }
    }

    void RSP::VMADL()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vs = get_vs();
        VectorRegister& vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        for (int i = 0; i < 8; i++)
        {
            accumulator_[i].Add(((static_cast<int64_t>(vs[i]) * vt[e[i]]) >> 16) & 0xFFFF);
        }

        for (int i = 0; i < 8; i++)
        {
            if (is_sign_extension(accumulator_[i].GetHigh(), accumulator_[i].GetMiddle()))
            {
                vd[i] = accumulator_[i].GetLow();
            }
            else if (accumulator_[i].GetHighSigned() < 0)
            {
                vd[i] = 0;
            }
            else
            {
                vd[i] = 0xFFFF;
            }
        }
    }

    void RSP::VMADM()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vs = get_vs();
        VectorRegister& vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        for (int i = 0; i < 8; i++)
        {
            accumulator_[i].Add(static_cast<int32_t>(static_cast<int16_t>(vs[i]) * vt[e[i]]));
        }

        for (int i = 0; i < 8; i++)
        {
            vd[i] = clamp_signed(accumulator_[i].GetSigned() >> 16);
        }
    }

    void RSP::VMADN()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vs = get_vs();
        VectorRegister& vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        for (int i = 0; i < 8; i++)
        {
            int64_t product = static_cast<int32_t>(vs[i] * static_cast<int16_t>(vt[e[i]]));
            accumulator_[i].Add(product);
        }

        for (int i = 0; i < 8; i++)
        {
            if (is_sign_extension(accumulator_[i].GetHigh(), accumulator_[i].GetMiddle()))
            {
                vd[i] = accumulator_[i].GetLow();
            }
            else if (accumulator_[i].GetHighSigned() < 0)
            {
                vd[i] = 0;
            }
            else
            {
                vd[i] = 0xFFFF;
            }
        }
    }

    void RSP::VMADH()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vs = get_vs();
        VectorRegister& vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        for (int i = 0; i < 8; i++)
        {
            accumulator_[i].Add(static_cast<int64_t>(static_cast<int32_t>(
                                    static_cast<int16_t>(vs[i]) * static_cast<int16_t>(vt[e[i]])))
                                << 16);
        }
        for (int i = 0; i < 8; i++)
        {
            vd[i] = clamp_signed(accumulator_[i].GetSigned() >> 16);
        }
    }

    void RSP::VADD()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vs = get_vs();
        VectorRegister vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        for (int i = 0; i < 8; i++)
        {
            int32_t result =
                static_cast<int16_t>(vs[i]) + static_cast<int16_t>(vt[e[i]]) + vco_.GetLow(i);
            accumulator_[i].SetLow(result);
            vd[i] = clamp_signed(result);
        }

        vco_.Clear();
    }

    void RSP::VADDC()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vs = get_vs();
        VectorRegister vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        vco_.Clear();

        for (int i = 0; i < 8; i++)
        {
            uint32_t result = vs[i] + vt[e[i]];
            accumulator_[i].SetLow(result);
            vco_.SetLow(i, (result >> 16) & 1);
        }

        for (int i = 0; i < 8; i++)
        {
            vd[i] = accumulator_[i].GetLow();
        }
    }

    void RSP::VSUB()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vs = get_vs();
        VectorRegister vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        for (int i = 0; i < 8; i++)
        {
            int32_t result =
                (static_cast<int16_t>(vs[i]) - static_cast<int16_t>(vt[e[i]]) - vco_.GetLow(i));
            accumulator_[i].SetLow(result);
            vd[i] = clamp_signed(result);
        }
        vco_.Clear();
    }

    void RSP::VSUBC()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vs = get_vs();
        VectorRegister vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        vco_.Clear();

        for (int i = 0; i < 8; i++)
        {
            uint32_t result = vs[i] - vt[e[i]];
            accumulator_[i].SetLow(result);
            vco_.SetLow(i, (result >> 16) & 1);
            vco_.SetHigh(i, result);
        }

        for (int i = 0; i < 8; i++)
        {
            vd[i] = accumulator_[i].GetLow();
        }
    }

    void RSP::VEQ()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vs = get_vs();
        VectorRegister vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        vcc_.Clear();

        for (int i = 0; i < 8; i++)
        {
            bool test = !vco_.GetHigh(i) && (vs[i] == vt[e[i]]);
            accumulator_[i].SetLow(test ? vs[i] : vt[e[i]]);
            vcc_.SetLow(i, test);
        }

        for (int i = 0; i < 8; i++)
        {
            vd[i] = accumulator_[i].GetLow();
        }

        vco_.Clear();
    }

    void RSP::VNE()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vs = get_vs();
        VectorRegister vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        vcc_.Clear();

        for (int i = 0; i < 8; i++)
        {
            int16_t vsi = static_cast<int16_t>(vs[i]);
            int16_t vti = static_cast<int16_t>(vt[e[i]]);
            bool test = (vsi != vti || (*vco_ & (0x100 << i)));
            accumulator_[i].SetLow(test ? vsi : vti);
            vcc_.SetLow(i, test);
        }

        for (int i = 0; i < 8; i++)
        {
            vd[i] = accumulator_[i].GetLow();
        }

        vco_.Clear();
    }

    void RSP::VGE()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vs = get_vs();
        VectorRegister vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        vcc_.Clear();

        for (int i = 0; i < 8; i++)
        {
            int16_t vsi = static_cast<int16_t>(vs[i]);
            int16_t vti = static_cast<int16_t>(vt[e[i]]);
            bool test = (vsi > vti || (vsi == vti && !(((*vco_ & (*vco_ >> 8)) >> i) & 1)));
            accumulator_[i].SetLow(test ? vsi : vti);
            vcc_.SetLow(i, test);
        }

        for (int i = 0; i < 8; i++)
        {
            vd[i] = accumulator_[i].GetLow();
        }

        vco_.Clear();
    }

    void RSP::VLT()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vs = get_vs();
        VectorRegister vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        vcc_.Clear();

        for (int i = 0; i < 8; i++)
        {
            bool test = vco_.GetHigh(i) && vco_.GetLow(i) && (vs[i] == vt[e[i]]);
            vcc_.SetLow(i, test || (static_cast<int16_t>(vs[i]) < static_cast<int16_t>(vt[e[i]])));
            accumulator_[i].SetLow(vcc_.GetLow(i) ? vs[i] : vt[e[i]]);
        }

        for (int i = 0; i < 8; i++)
        {
            vd[i] = accumulator_[i].GetLow();
        }

        vco_.Clear();
    }

    // Stolen from Dillon for now
    uint32_t rcp(int32_t sinput)
    {
        int32_t mask = sinput >> 31;
        int32_t input = sinput ^ mask;
        if (sinput > INT16_MIN)
        {
            input -= mask;
        }
        if (input == 0)
        {
            return 0x7FFFFFFF;
        }
        else if (sinput == INT16_MIN)
        {
            return 0xFFFF0000;
        }

        uint32_t shift = hydra::clz<uint32_t>(input);
        uint64_t dinput = (uint64_t)input;
        uint32_t index = ((dinput << shift) & 0x7FC00000) >> 22;

        int32_t result = RCP_TABLE[index];
        result = (0x10000 | result) << 14;
        result = (result >> (31 - shift)) ^ mask;
        return result;
    }

    // Stolen from Dillon for now
    uint32_t rsq(uint32_t input)
    {
        if (input == 0)
        {
            return 0x7FFFFFFF;
        }
        else if (input == 0xFFFF8000)
        {
            return 0xFFFF0000;
        }
        else if (input > 0xFFFF8000)
        {
            input--;
        }

        int32_t sinput = input;
        int32_t mask = sinput >> 31;
        input ^= mask;

        int shift = hydra::clz<uint32_t>(input) + 1;

        int index = (((input << shift) >> 24) | ((shift & 1) << 8));
        uint32_t rom = (((uint32_t)RSQ_TABLE[index]) << 14);
        int r_shift = ((32 - shift) >> 1);
        uint32_t result = (0x40000000 | rom) >> r_shift;

        return result ^ mask;
    }

    void RSP::VRSQL()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        int32_t input;
        int se = vuinstr.element & 0b111;
        int de = vuinstr.vs & 0b111;

        if (div_in_ready_)
        {
            input = (static_cast<int32_t>(div_in_) << 16) | vt[se];
        }
        else
        {
            input = static_cast<int16_t>(vt[se]);
        }

        uint32_t result = rsq(input);
        div_out_ = result >> 16;
        div_in_ = 0;
        div_in_ready_ = false;

        for (int i = 0; i < 8; i++)
        {
            accumulator_[i].SetLow(vt[e[i]]);
        }

        vd[de] = result & 0xFFFF;
    }

    void RSP::VRSQ()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        int se = vuinstr.element & 0b111;
        int de = vuinstr.vs & 0b111;
        int16_t input = vt[se];
        int result = rsq(input);

        div_out_ = result >> 16;

        for (int i = 0; i < 8; i++)
        {
            accumulator_[i].SetLow(vt[e[i]]);
        }

        vd[de] = result & 0xFFFF;
    }

    void RSP::VRCP()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        int se = vuinstr.element & 0b111;
        int de = vuinstr.vs & 0b111;
        int16_t input = vt[se];
        int result = rcp(input);

        div_out_ = result >> 16;

        for (int i = 0; i < 8; i++)
        {
            accumulator_[i].SetLow(vt[e[i]]);
        }

        vd[de] = result & 0xFFFF;
    }

    void RSP::VRCPL()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        int32_t input;
        int se = vuinstr.element & 0b111;
        int de = vuinstr.vs & 0b111;

        if (div_in_ready_)
        {
            input = (static_cast<int32_t>(div_in_) << 16) | vt[se];
        }
        else
        {
            input = static_cast<int16_t>(vt[se]);
        }

        int32_t result = rcp(input);
        div_out_ = result >> 16;
        div_in_ = 0;
        div_in_ready_ = false;

        for (int i = 0; i < 8; i++)
        {
            accumulator_[i].SetLow(vt[e[i]]);
        }

        vd[de] = result & 0xFFFF;
    }

    void RSP::VRCPH()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        int se = vuinstr.element & 0b111;
        int de = vuinstr.vs & 0b111;

        for (int i = 0; i < 8; i++)
        {
            accumulator_[i].SetLow(vt[e[i]]);
        }

        div_in_ = vt[se];
        vd[de] = div_out_;

        div_in_ready_ = true;
    }

    void RSP::VMOV()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vt = get_vt();
        const Elements& e = elements[vuinstr.element];
        uint8_t element = vuinstr.element & 0b1111;
        uint8_t se;

        switch (element)
        {
            case 0:
            case 1:
                se = (element & 0b000) | (vuinstr.vs & 0b111);
                break;
            case 2:
            case 3:
                se = (element & 0b001) | (vuinstr.vs & 0b110);
                break;
            case 4:
            case 5:
            case 6:
            case 7:
                se = (element & 0b011) | (vuinstr.vs & 0b100);
                break;
            default: // 8 ... 15
                se = (element & 0b111) | (vuinstr.vs & 0b000);
                break;
        }

        uint16_t vti = vt[e[se]];
        vd[vuinstr.vs & 0b111] = vti;

        for (int i = 0; i < 8; i++)
        {
            accumulator_[i].SetLow(vt[e[i]]);
        }
    }

    void RSP::VMRG()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vs = get_vs();
        VectorRegister& vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        for (int i = 0; i < 8; i++)
        {
            accumulator_[i].SetLow(vcc_.GetLow(i) ? vs[i] : vt[e[i]]);
        }

        for (int i = 0; i < 8; i++)
        {
            vd[i] = accumulator_[i].GetLow();
        }

        vco_.Clear();
    }

    void RSP::VCH()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vs = get_vs();
        VectorRegister& vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        for (int i = 0; i < 8; i++)
        {
            int16_t vsi = static_cast<int16_t>(vs[i]);
            int16_t vti = static_cast<int16_t>(vt[e[i]]);

            if ((vsi ^ vti) < 0)
            {
                int16_t sum = vsi + vti;
                accumulator_[i].SetLow(sum <= 0 ? -vti : vsi);
                vcc_.SetLow(i, sum <= 0);
                vcc_.SetHigh(i, vti < 0);
                vco_.SetLow(i, true);
                vco_.SetHigh(i, sum != 0 && (vs[i] != (vt[e[i]] ^ 0xFFFF)));
                vce_.Set(i, sum == -1);
            }
            else
            {
                int16_t dif = vsi - vti;
                accumulator_[i].SetLow(dif >= 0 ? vti : vsi);
                vcc_.SetLow(i, vti < 0);
                vcc_.SetHigh(i, dif >= 0);
                vco_.SetLow(i, false);
                vco_.SetHigh(i, dif != 0 && (vs[i] != (vt[e[i]] ^ 0xFFFF)));
                vce_.Set(i, false);
            }
        }

        for (int i = 0; i < 8; i++)
        {
            vd[i] = accumulator_[i].GetLow();
        }
    }

    void RSP::VCR()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vs = get_vs();
        VectorRegister& vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        for (int i = 0; i < 8; i++)
        {
            bool sign = (0x8000 & (vs[i] ^ vt[e[i]])) != 0;
            uint16_t vti = sign ? ~vt[e[i]] : vt[e[i]];

            bool gte =
                static_cast<int16_t>(vt[e[i]]) <= static_cast<int16_t>(sign ? 0xFFFF : vs[i]);
            bool lte = (((sign ? vs[i] : 0) + vt[e[i]]) & 0x8000) != 0;

            bool check = sign ? lte : gte;
            uint16_t result = check ? vti : vs[i];
            accumulator_[i].SetLow(result);

            vcc_.SetHigh(i, gte);
            vcc_.SetLow(i, lte);
        }

        for (int i = 0; i < 8; i++)
        {
            vd[i] = accumulator_[i].GetLow();
        }

        vco_.Clear();
        vce_.Clear();
    }

    void RSP::VCL()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vs = get_vs();
        VectorRegister& vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        for (int i = 0; i < 8; i++)
        {
            if (vco_.GetLow(i))
            {
                if (vco_.GetHigh(i))
                {
                    accumulator_[i].SetLow(vcc_.GetLow(i) ? -vt[e[i]] : vs[i]);
                }
                else
                {
                    uint16_t result = 0;
                    bool overflow = hydra::add_overflow(vs[i], vt[e[i]], result);
                    if (vce_.Get(i))
                    {
                        vcc_.SetLow(i, !result || !overflow);
                        accumulator_[i].SetLow(vcc_.GetLow(i) ? -vt[e[i]] : vs[i]);
                    }
                    else
                    {
                        vcc_.SetLow(i, !result && !overflow);
                        accumulator_[i].SetLow(vcc_.GetLow(i) ? -vt[e[i]] : vs[i]);
                    }
                }
            }
            else
            {
                if (vco_.GetHigh(i))
                {
                    accumulator_[i].SetLow(vcc_.GetHigh(i) ? vt[e[i]] : vs[i]);
                }
                else
                {
                    vcc_.SetHigh(i, vs[i] >= vt[e[i]]);
                    accumulator_[i].SetLow(vcc_.GetHigh(i) ? vt[e[i]] : vs[i]);
                }
            }
        }

        for (int i = 0; i < 8; i++)
        {
            vd[i] = accumulator_[i].GetLow();
        }

        vco_.Clear();
        vce_.Clear();
    }

    void RSP::VABS()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vs = get_vs();
        VectorRegister& vt = get_vt();
        const Elements& e = elements[vuinstr.element];
        std::array<bool, 8> edge_case{};

        for (int i = 0; i < 8; i++)
        {
            uint16_t data = 0;
            if (static_cast<int16_t>(vs[i]) < 0)
            {
                if (vt[e[i]] == 0x8000)
                {
                    edge_case[i] = true;
                }
                data = -vt[e[i]];
            }
            else if (static_cast<int16_t>(vs[i]) > 0)
            {
                data = vt[e[i]];
            }
            accumulator_[i].SetLow(data);
        }

        for (int i = 0; i < 8; i++)
        {
            vd[i] = accumulator_[i].GetLow();
            if (edge_case[i]) [[unlikely]]
            {
                vd[i] = 0x7FFF;
            }
        }
    }

    void RSP::VZERO()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vs = get_vs();
        VectorRegister& vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        for (int i = 0; i < 8; i++)
        {
            accumulator_[i].SetLow(vt[e[i]] + vs[i]);
        }

        for (int i = 0; i < 8; i++)
        {
            vd[i] = 0;
        }
    }

    void RSP::VMULQ()
    {
        VectorRegister& vd = get_vd();
        VectorRegister& vs = get_vs();
        VectorRegister vt = get_vt();
        const Elements& e = elements[vuinstr.element];

        for (int i = 0; i < 8; i++)
        {
            int32_t product = static_cast<int16_t>(vs[i]) * static_cast<int16_t>(vt[e[i]]);
            if (product < 0)
            {
                product += 31;
            }

            accumulator_[i].SetHigh(product >> 16);
            accumulator_[i].SetMiddle(product);
            accumulator_[i].SetLow(0);

            vd[i] = clamp_signed(product >> 1) & ~0b1111;
        }
    }

    void RSP::VMACQ()
    {
        VectorRegister& vd = get_vd();

        for (int i = 0; i < 8; i++)
        {
            int32_t product = (accumulator_[i].GetHigh() << 16) | accumulator_[i].GetMiddle();
            if (product < 0 && !(product & 1 << 5))
            {
                product += 32;
            }
            else if (product >= 32 && !(product & 1 << 5))
            {
                product -= 32;
            }
            accumulator_[i].SetHigh(product >> 16);
            accumulator_[i].SetMiddle(product);

            vd[i] = clamp_signed(product >> 1) & ~0b1111;
        }
    }

    void RSP::VNOP() {}

    void RSP::ERROR()
    {
        Logger::Warn("Ran ERROR instruction: {:08x}", instruction_.full);
    }

    void RSP::SPECIAL()
    {
        (special_table_[instruction_.RType.func])(this);
    }

    void RSP::REGIMM()
    {
        (regimm_table_[instruction_.RType.rt])(this);
    }

    void RSP::s_SLL()
    {
        rdreg.UW = rtreg.UW << saval;
    }

    void RSP::s_SLLV()
    {
        rdreg.UW = rtreg.UW << (rsreg.UW & 0b11111);
    }

    void RSP::s_SRL()
    {
        rdreg.UW = rtreg.UW >> saval;
    }

    void RSP::s_SRA()
    {
        rdreg.W = rtreg.W >> saval;
    }

    void RSP::s_SRAV()
    {
        rdreg.W = rtreg.W >> (rsreg.UW & 0b11111);
    }

    void RSP::s_SRLV()
    {
        rdreg.UW = rtreg.UW >> (rsreg.UW & 0b11111);
    }

    void RSP::s_JR()
    {
        auto jump_addr = rsreg.UW;
        branch_to(jump_addr);
    }

    void RSP::s_JALR()
    {
        auto jump_addr = rsreg.UW;
        link_register(instruction_.RType.rd);
        branch_to(jump_addr);
    }

    void RSP::s_ADDU()
    {
        rdreg.UW = rtreg.UW + rsreg.UW;
    }

    void RSP::s_SUBU()
    {
        rdreg.UW = rsreg.UW - rtreg.UW;
    }

    void RSP::s_SLT()
    {
        rdreg.UW = rsreg.W < rtreg.W;
    }

    void RSP::s_SLTU()
    {
        rdreg.UW = rsreg.UW < rtreg.UW;
    }

    void RSP::s_BREAK()
    {
        status_.halt = true;
        status_.broke = true;
        if (status_.intr_break)
        {
            Logger::Debug("Raising SP interrupt");
            interrupt_callback_(true);
        }
    }

    void RSP::s_AND()
    {
        rdreg.UW = rtreg.UW & rsreg.UW;
    }

    void RSP::s_OR()
    {
        rdreg.UW = rtreg.UW | rsreg.UW;
    }

    void RSP::s_XOR()
    {
        rdreg.UW = rtreg.UW ^ rsreg.UW;
    }

    void RSP::s_NOR()
    {
        rdreg.UW = ~(rtreg.UW | rsreg.UW);
    }

    void RSP::r_BGEZ()
    {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch(rsreg.W >= 0, pc_ + seoffset);
    }

    void RSP::r_BLTZ()
    {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch(rsreg.W < 0, pc_ + seoffset);
    }

    void RSP::r_BGEZAL()
    {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch(rsreg.W >= 0, pc_ + seoffset);
        link_register(31);
    }

    void RSP::r_BLTZAL()
    {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch(rsreg.W < 0, pc_ + seoffset);
        link_register(31);
    }

    void RSP::J()
    {
        auto jump_addr = instruction_.JType.target << 2;
        branch_to(jump_addr);
    }

    void RSP::JAL()
    {
        link_register(31);
        J();
    }

    void RSP::BEQ()
    {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch(rsreg.UW == rtreg.UW, pc_ + seoffset);
    }

    void RSP::BNE()
    {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch(rsreg.UW != rtreg.UW, pc_ + seoffset);
    }

    void RSP::BLEZ()
    {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch(rsreg.W <= 0, pc_ + seoffset);
    }

    void RSP::BGTZ()
    {
        int16_t offset = immval << 2;
        int32_t seoffset = offset;
        conditional_branch(rsreg.W > 0, pc_ + seoffset);
    }

    void RSP::ADDI()
    {
        int32_t seimm = static_cast<int16_t>(immval);
        int32_t result = 0;
        bool overflow = hydra::add_overflow(rsreg.W, seimm, result);
        rtreg.W = result;

        if (overflow)
        {
            Logger::Fatal("RSP: ADDI overflowed");
        }
    }

    void RSP::ADDIU()
    {
        int32_t seimm = static_cast<int16_t>(immval);
        int32_t result = rsreg.W + seimm;
        rtreg.UW = result;
    }

    void RSP::SLTI()
    {
        rtreg.UW = rsreg.W < seimmval;
    }

    void RSP::SLTIU()
    {
        rtreg.UW = rsreg.UW < static_cast<uint32_t>(seimmval);
    }

    void RSP::ANDI()
    {
        rtreg.UW = rsreg.UW & immval;
    }

    void RSP::ORI()
    {
        rtreg.UW = rsreg.UW | immval;
    }

    void RSP::XORI()
    {
        rtreg.UW = rsreg.UW ^ immval;
    }

    void RSP::LUI()
    {
        int32_t imm = immval << 16;
        rtreg.UW = imm;
    }

    void RSP::COP0()
    {
        switch (instruction_.RType.rs)
        {
            // MFC0
            case 0:
                rtreg.UW = read_hwio(static_cast<RSPHWIO>(instruction_.RType.rd));
                break;
            // MTC0
            case 4:
                return write_hwio(static_cast<RSPHWIO>(instruction_.RType.rd), rtreg.UW);
            default:
            {
                Logger::Fatal("RSP: COP0 unimplemented function: {}",
                              static_cast<uint8_t>(instruction_.RType.rs));
            }
        }
    }

    void RSP::COP1()
    {
        Logger::Warn("RSP: COP1 not implemented");
    }

    void RSP::COP2()
    {
        switch (instruction_.WCType.base)
        {
            case 0:
                return MFC2();
            case 2:
                return CFC2();
            case 4:
                return MTC2();
            case 6:
                return CTC2();
            default:
            {
                (vu_instruction_table_[instruction_.FType.func])(this);
                break;
            }
        }
    }

    void RSP::MFC2()
    {
        gpr_regs_[instruction_.WCType.vt].UW =
            get_lane(instruction_.WCType.opcode, instruction_.WCType.element);
    }

    void RSP::CFC2()
    {
        gpr_regs_[instruction_.WCType.vt].UW = get_control(instruction_.WCType.opcode);
    }

    void RSP::MTC2()
    {
        set_lane(instruction_.WCType.opcode, instruction_.WCType.element,
                 gpr_regs_[instruction_.WCType.vt].UW);
    }

    void RSP::CTC2()
    {
        set_control(instruction_.WCType.opcode, gpr_regs_[instruction_.WCType.vt].UW);
    }

    void RSP::LB()
    {
        int16_t offset = immval;
        int32_t seoffset = offset;
        rtreg.W = static_cast<int8_t>(load_byte(seoffset + rsreg.UW));
    }

    void RSP::LH()
    {
        int16_t offset = immval;
        int32_t seoffset = offset;
        rtreg.W = static_cast<int16_t>(load_halfword(seoffset + rsreg.UW));
    }

    void RSP::LW()
    {
        int16_t offset = immval;
        int32_t seoffset = offset;
        rtreg.W = load_word(seoffset + rsreg.UW);
    }

    void RSP::LBU()
    {
        int16_t offset = immval;
        int32_t seoffset = offset;
        rtreg.UW = load_byte(seoffset + rsreg.UW);
    }

    void RSP::LHU()
    {
        int16_t offset = immval;
        int32_t seoffset = offset;
        rtreg.UW = load_halfword(seoffset + rsreg.UW);
    }

    void RSP::LWU()
    {
        int16_t offset = immval;
        int32_t seoffset = offset;
        rtreg.UW = load_word(seoffset + rsreg.UW);
    }

    void RSP::SB()
    {
        int16_t offset = immval;
        int32_t seoffset = offset;
        auto address = seoffset + rsreg.UW;
        store_byte(address, rtreg.UB._0);
    }

    void RSP::SH()
    {
        int16_t offset = immval;
        int32_t seoffset = offset;
        auto address = seoffset + rsreg.UW;
        store_halfword(address, rtreg.UH._0);
    }

    void RSP::SW()
    {
        int16_t offset = immval;
        int32_t seoffset = offset;
        auto address = seoffset + rsreg.UW;
        store_word(address, rtreg.UW);
    }

    void RSP::CACHE()
    {
        Logger::Fatal("RSP: CACHE not implemented");
    }

} // namespace hydra::N64

#undef vuinstr
#undef rdreg
#undef rsreg
#undef rtreg
#undef saval
#undef immval
#undef seimmval
