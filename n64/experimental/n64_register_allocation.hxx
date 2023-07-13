#pragma once

#include <xbyak/xbyak.h>
#include <array>

using namespace Xbyak;
using namespace Xbyak::util;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
static_assert(false && "Whoopsies");
#else
constexpr Reg64 RegisterPointer = rbp;
constexpr Reg64 PcPointer = rbx;
constexpr std::array<Reg64, 8> allocateableRegisters = {r8,  r9,  r10, r11, r12, r13, r14, r15};
constexpr std::array<Reg64, 4> Volatiles = {r8, r9, r10, r11};
constexpr std::array<Reg64, 4> NonVolatiles = {r12, r13, r14, r15};
constexpr Reg64 arg1 = rdi;
constexpr Reg64 arg2 = rsi;
constexpr Reg64 arg3 = rdx;
constexpr Reg64 arg4 = rcx;
#endif