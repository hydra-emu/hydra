#pragma once
#ifndef TKP_TOOLS_GBADDR_H
#define TKP_TOOLS_GBADDR_H
#include "disassembly_instr.h"
#include <cstdint>
using DisInstr = TKPEmu::Tools::DisInstr;

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