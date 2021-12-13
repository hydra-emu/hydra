#pragma once
#ifndef TKP_TOOLS_GBADDR_H
#define TKP_TOOLS_GBADDR_H
#include "../include/disassembly_instr.h"
#include "gb_breakpoint.h"
#include <cstdint>
using RegisterType = uint8_t;
using BigRegisterType = uint16_t;
using DisInstr = TKPEmu::Tools::DisInstr;
using GBBPArguments = TKPEmu::Gameboy::Utils::GBBPArguments;
constexpr size_t LogTypeSize = 17;
enum class LogType {
    InstrName, InstrNum,
    A, B, C, D, 
    E, F, H, L,
    PC, SP, LY,
    IF, IE, IME, HALT
};
enum LCDCFlag {
    BG_ENABLE = 1 << 0,
    OBJ_ENABLE = 1 << 1,
    OBJ_SIZE = 1 << 2,
    BG_TILEMAP = 1 << 3,
    BG_TILES = 1 << 4,
    WND_ENABLE = 1 << 5,
    WND_TILEMAP = 1 << 6,
    LCD_ENABLE = 1 << 7
};
enum STATFlag {
    MODE = 0b11,
    COINCIDENCE = 1 << 2,
    MODE0_INTER = 1 << 3,
    MODE1_INTER = 1 << 4,
    MODE2_INTER = 1 << 5,
    COINC_INTER = 1 << 6
};
enum IFInterrupt {
    VBLANK = 1 << 0,
    LCDSTAT = 1 << 1,
    TIMER = 1 << 2,
    SERIAL = 1 << 3,
    JOYPAD = 1 << 4
};
// Used in the map below, to compare rom hashes with expected results after
// a hardcoded number of clocks
using Hash = std::string;
struct ExpectedResult {
    unsigned long long Clocks;
    // Represents the hash of the screenshot taken after Clocks
    Hash ExpectedHash;
};
// This map helps with quality assurance, we can check multiple test roms
// at once and compare their finished hashes with these known good results
const static std::unordered_map<Hash, ExpectedResult> PassedTestMap {
    // blargg
    // 01-special
    { "7d95af543a521ed036dc85f6f232d103", { 1'300'000, "42d5abde92c2678617996dd8f782989c" } },
    // 02-interrupts
    { "d36a85bb94d4c1b373c0e7be0f6f0971", { 200'000, "0f384cd6115fd9b2c33f7d9fc42200b5" } },
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
    
};
constexpr static std::array<LogType, LogTypeSize> LogTypeMap {
    LogType::InstrName, LogType::InstrNum,
    LogType::A, LogType::B, LogType::C, LogType::D,
    LogType::E, LogType::F, LogType::H, LogType::L,
    LogType::PC, LogType::SP, LogType::LY, LogType::IF,
    LogType::IE, LogType::IME, LogType::HALT
};
const static std::array<std::string, LogTypeSize> LogTypeNames {
    "Instr. ", "Opcode ",
    "A      ", "B      ", "C      ", "D      ",
    "E      ", "F      ", "H      ", "L      ",
    "PC     ", "SP     ", "LY     ", "IF     ",
    "IE     ", "IME    ", "HALT   ",
};
constexpr std::array<uint8_t, 0x100> InstrTimes = {
    0, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0,
    0, 2, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0,
    1, 2, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0,
    1, 2, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 2, 2, 2, 0, 1, 0, 0, 0, 2, 1, 2, 2, 2, 0,
    0, 0, 2, 0, 2, 0, 1, 0, 0, 0, 2, 0, 2, 0, 1, 1,
    0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 2,
    1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0
};
constexpr auto cl_white = 0;
constexpr auto cl_lgray = 1;
constexpr auto cl_dgray = 2;
constexpr auto cl_black = 3;
constexpr auto addr_joy = 0xFF00;
// Serial registers
constexpr auto addr_std = 0xFF01;
constexpr auto addr_stc = 0xFF02;
// Timer registers
constexpr auto addr_div = 0xFF04;
constexpr auto addr_tim = 0xFF05;
constexpr auto addr_tma = 0xFF06;
constexpr auto addr_tac = 0xFF07;
// Interrupt flag
constexpr auto addr_ifl = 0xFF0F;
// Sound registers
constexpr auto addr_s1s = 0xFF10;
constexpr auto addr_s3e = 0xFF1A;
constexpr auto addr_s3o = 0xFF1C;
constexpr auto addr_s4l = 0xFF20;
constexpr auto addr_s4c = 0xFF23;
constexpr auto addr_snd = 0xFF26;
// PPU & OAM related registers
constexpr auto addr_lcd = 0xFF40;
constexpr auto addr_sta = 0xFF41;
constexpr auto addr_lly = 0xFF44;
constexpr auto addr_dma = 0xFF46;
constexpr auto addr_bgp = 0xFF47;
constexpr auto addr_ob0 = 0xFF48;
constexpr auto addr_ob1 = 0xFF49;

constexpr auto addr_if = 0xFF0F;
constexpr auto addr_ie = 0xFFFF;

// CPU flag masks and shifts
constexpr uint8_t FLAG_ZERO_MASK = 0b10000000;
constexpr uint8_t FLAG_NEG_MASK = 0b01000000;
constexpr uint8_t FLAG_HCARRY_MASK = 0b00100000;
constexpr uint8_t FLAG_CARRY_MASK = 0b00010000;
constexpr uint8_t FLAG_EMPTY_MASK = 0b00000000;
constexpr uint8_t FLAG_ZERO_SHIFT = 7;
constexpr uint8_t FLAG_NEG_SHIFT = 6;
constexpr uint8_t FLAG_HCARRY_SHIFT = 5;
constexpr uint8_t FLAG_CARRY_SHIFT = 4;
#endif