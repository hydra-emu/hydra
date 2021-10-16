#ifndef TKP_CPU_CONST_H
#define TKP_CPU_CONST_H
#include <cstdint>
constexpr uint8_t FLAG_ZERO_MASK    = 0b10000000;
constexpr uint8_t FLAG_NEG_MASK     = 0b01000000;
constexpr uint8_t FLAG_HCARRY_MASK  = 0b00100000;
constexpr uint8_t FLAG_CARRY_MASK   = 0b00010000;
constexpr uint8_t FLAG_EMPTY_MASK   = 0b00000000;
constexpr uint8_t FLAG_ZERO_SHIFT   = 7;
constexpr uint8_t FLAG_NEG_SHIFT    = 6;
constexpr uint8_t FLAG_HCARRY_SHIFT = 5;
constexpr uint8_t FLAG_CARRY_SHIFT  = 4;
#endif