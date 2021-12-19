#include "../include/emulator_results.h"
namespace TKPEmu::Testing {
const std::unordered_map<Hash, ExpectedResult> QA::PassedTestMap = 
{
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    //      rom hash                       clocks to            expected                    rom name        //
    //                                    complete test          result                                     //
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Gameboy
    // blargg
    { "7d95af543a521ed036dc85f6f232d103", { 1'300'000, "42d5abde92c2678617996dd8f782989c", "cpu_instrs/01-special" } },
    { "d36a85bb94d4c1b373c0e7be0f6f0971", { 300'000, "0f384cd6115fd9b2c33f7d9fc42200b5", "cpu_instrs02-interrupts" } },
    { "5bccf6b03f661d92b2903694d458510c", { 1'150'000, "3caaa1d70619add931ecfa9e88e3a7ff", "cpu_instrs/03-op sp,hl" } },
    { "e97a5202d7725a3caaf3495e559f2e98", { 1'400'000, "cccde7fb4b57b51d73147233e2789d0e", "cpu_instrs/04-op r,imm" } },
    { "43fc8bfc94938b42d8ecc9ea8b6b811a", { 1'900'000, "2d0258217d9411ae7dc9390b4022e7fa", "cpu_instrs/05-op rp" } },
    { "24da4eed7ef73ec32aae5ffd50ebec55", { 300'000, "45f17918f8da5383982f33eda50b3714", "cpu_instrs/06-ld r,r" } },
    { "6dbf4e754ef2f844246fd08718d1c377", { 400'000, "c81680b1a44aab843cea7936de3da10f", "cpu_instrs/07-jr,jp,call,ret,rst" } },
    { "c21ddacbfa44d61919c8e2d6c3e7d26e", { 350'000, "820df31460734f4451ef46673a5e297c", "cpu_instrs/08-misc instrs" } },
    { "e4c4dd4eebad0c9d6f2ef575331c3aee", { 4'500'000, "7a0cae7fe13aba1179096a74161dbd81", "cpu_instrs/09-op r,r" } },
    { "64632849778ee83ae85db8bf68c84ebc", { 7'000'000, "56d069d71d8b2114149a6a605df2ef28", "cpu_instrs/10-bit ops" } },
    { "6e64346be4d7ccf26f53de105d6cb5f6", { 7'500'000, "3215a908fc7e7aac137f20b9dec08e9e", "cpu_instrs/11-op a,(hl)" } },
    { "b417d5d06c3382ab5836b5d365184f36", { 350'000, "89c5c02898f9299e22457dc47cab40a0", "instr_timing" } },
    { "9537182264201f75611fc96a1de0f086", { 258'877, "bd9fc5fdc70e82b3eb643c4c7dacd4ab", "mem_timing/01-read_timing"} },
    { "d5cf8017991700f267b7b753579cc773", { 222'676, "bc9ff8c11b34a1c09800f79349ede2bf", "mem_timing/02-write_timing" } },
    // Gekkio
    { "f9bf05a2048eb83ce2d25eaa15e59b36", { 193265, "fa43d52c2e302bfff8c0c55c50e9401b", "acceptance/boot_regs-dmgABC" } },
    { "6bf0f67b415799534dc64e9c60521401", { 176006, "8c77db0a6370393f573540736131d2ef", "acceptance/div_timing" } },
    { "e5a7c1bb00c125db63974ec05b23de33", { 298225, "2eee4e1b02e5aeb4c4c6ae686c8ea07e", "acceptance/ei_sequence" } },
    { "82f55f1beaca6b7efc744b70f36de94e", { 175889, "12da5b255c481dba0f086a2b20001ae1", "acceptance/ei_timing" } },
    { "8ae8e8bfc9985cae9836a76dd4ff41d4", { 175908, "f0553923b341e4e57f091f1ce8cc8cf0", "acceptance/halt_ime0_ei" } },
    { "603ec0145d94806c5ac2e0e150d0f8a7", { 193386, "1975dc57ea974f8b92d14b07f1a9a584", "acceptance/halt_ime1_timing" } },
    { "612dff157f718c752aee5fe4d89f806c", { 175867, "6991e00223ecbd57308a7a7116c328f4", "acceptance/rapid_di_ei" } },
    { "14e4eed76f51691e27279b74c8254a29", { 175872, "6090b1beba327d9d1934088dc548de60", "acceptance/reti_intr_timing" } },
    { "66bafbd73b021d45b174afce483a5614", { 165116, "f0553923b341e4e57f091f1ce8cc8cf0", "acceptance/bits/mem_oam" } },  
    { "04abf53326dec066aad078f4da4ccc33", { 228312, "c1c98a124d8bc1988d22265f64a6209d", "acceptance/bits/reg_f" } },
    { "e905eb9e6e55a5624a915c9a1e81b5a1", { 208289, "f0553923b341e4e57f091f1ce8cc8cf0", "acceptance/bits/unused_hwio-GS" } },
    { "8fa02c48b75200ff96d445ea3bd5f3c3", { 325050, "f0553923b341e4e57f091f1ce8cc8cf0", "acceptance/instrs/daa" } },
    { "ecb8b4e807ee00e8d7d4affbec330845", { 200209, "5100702f820a1efc1f76e0c5e98084e3", "acceptance/oam_dma/reg_read" } },
    { "c57eada752f746347951f79c828391b9", { 508381, "f0553923b341e4e57f091f1ce8cc8cf0", "acceptance/timer/div_write" } },
    { "d73c739b0950f1720799098c21e8dafd", { 228639, "90472bf97c60517ccae5b2fe2c5c205a", "acceptance/timer/tim_00" } },
    { "22538d107bcd8b989e534e5bd06bfeac", { 193385, "8143d0e2e5f7dcc7015e132d1d1bc9b0", "acceptance/timer/tim_01" } },
    { "f82cb93bb388d1ccfbc125189db2337a", { 188733, "90472bf97c60517ccae5b2fe2c5c205a", "acceptance/timer/tim_10" } },
    { "94e8634725beab0e49c9a956ded2ebe5", { 175968, "90472bf97c60517ccae5b2fe2c5c205a", "acceptance/timer/tim_11" } },
    { "b8ce040bf3dc44ae93dfdd08455db5a3", { 210951, "39d1c2b42ed461c1422e8826143f77f8", "acceptance/timer/tima_reload" } },
    { "31d2ad77d05566b1dcb92d7ff7232767", { 1614439, "f0553923b341e4e57f091f1ce8cc8cf0", "emulator-only/mbc1/bits_bank1" } },
    { "d7c8eb26b8c276f0d13e2a272a17c308", { 1677811, "f0553923b341e4e57f091f1ce8cc8cf0", "emulator-only/mbc1/bits_bank2" } },
    { "d807288f6cfe34225989dd0d9293043e", { 1614699, "f0553923b341e4e57f091f1ce8cc8cf0", "emulator-only/mbc1/bits_mode" } },
    { "37666e894123e433ad28039992a0dc39", { 3379433, "f0553923b341e4e57f091f1ce8cc8cf0", "emulator-only/mbc1/bits_ramg" } },
    { "20bb5cd9b26696955e2304c721d658b9", { 752664, "f0553923b341e4e57f091f1ce8cc8cf0", "emulator-only/mbc1/ram_256kb" } },
    { "cc197e887003211e175ba5c2fa1bf0bd", { 557442, "f0553923b341e4e57f091f1ce8cc8cf0", "emulator-only/mbc1/ram_64kb" } },
    { "c4dc3398b815a34591fed91a1d5329b1", { 207111, "f0553923b341e4e57f091f1ce8cc8cf0", "emulator-only/mbc1/rom_16MB" } },
    { "c4620fa2f6a36b8264fb7c876a627fd6", { 172157, "f0553923b341e4e57f091f1ce8cc8cf0", "emulator-only/mbc1/rom_4MB" } },
    { "2435d3266665065e6b2a5047d1fad186", { 172157, "f0553923b341e4e57f091f1ce8cc8cf0", "emulator-only/mbc1/rom_8MB" } }
};
}