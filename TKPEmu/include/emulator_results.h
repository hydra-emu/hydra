#ifndef TKPEMU_RESULTS_H
#define TKPEMU_RESULTS_H
// This map helps with quality assurance, we can check multiple test roms
// at once and compare their finished hashes with these known good results
const static std::unordered_map<Hash, ExpectedResult> PassedTestMap {
    /// Gameboy
    // blargg
    // 01-special
    { "7d95af543a521ed036dc85f6f232d103", { 1'300'000, "42d5abde92c2678617996dd8f782989c" } },
    // 02-interrupts
    { "d36a85bb94d4c1b373c0e7be0f6f0971", { 300'000, "0f384cd6115fd9b2c33f7d9fc42200b5" } },
    // 03-op sp,hl
    { "5bccf6b03f661d92b2903694d458510c", { 1'150'000, "3caaa1d70619add931ecfa9e88e3a7ff" } },
    // 04-op r,imm
    { "e97a5202d7725a3caaf3495e559f2e98", { 1'400'000, "cccde7fb4b57b51d73147233e2789d0e" } },
    // 05-op rp
    { "43fc8bfc94938b42d8ecc9ea8b6b811a", { 1'900'000, "2d0258217d9411ae7dc9390b4022e7fa" } },
    // 06-ld r,r
    { "24da4eed7ef73ec32aae5ffd50ebec55", { 300'000, "45f17918f8da5383982f33eda50b3714" } },
    // 07-jr,jp,call,ret,rst
    { "6dbf4e754ef2f844246fd08718d1c377", { 400'000, "c81680b1a44aab843cea7936de3da10f" } },
    // 08-misc instrs
    { "c21ddacbfa44d61919c8e2d6c3e7d26e", { 350'000, "820df31460734f4451ef46673a5e297c" } },
    // 09-op r,r
    { "e4c4dd4eebad0c9d6f2ef575331c3aee", { 4'500'000, "7a0cae7fe13aba1179096a74161dbd81" } },
    // 10-bit ops
    { "64632849778ee83ae85db8bf68c84ebc", { 7'000'000, "56d069d71d8b2114149a6a605df2ef28" } },
    // 11-op a,(hl)
    { "6e64346be4d7ccf26f53de105d6cb5f6", { 7'500'000, "3215a908fc7e7aac137f20b9dec08e9e" } },
    // instr_timing
    { "b417d5d06c3382ab5836b5d365184f36", { 350'000, "89c5c02898f9299e22457dc47cab40a0" } },
};
#endif