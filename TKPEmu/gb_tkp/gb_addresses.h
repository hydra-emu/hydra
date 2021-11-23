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
constexpr size_t LogTypeSize = 10;
enum class LogType {
    A, B, C, D, 
    E, F, H, L,
    PC, SP,
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
constexpr static std::array<LogType, LogTypeSize> LogTypeMap {
    LogType::A, LogType::B, LogType::C, LogType::D,
    LogType::E, LogType::F, LogType::H, LogType::L,
    LogType::PC, LogType::SP
};
const static std::array<std::string, LogTypeSize> LogTypeNames {
    "A   ", "B   ", "C   ", "D   ",
    "E   ", "F   ", "H   ", "L   ",
    "PC  ", "SP  "
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