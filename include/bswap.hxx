#pragma once

#include <cstdint>

inline uint16_t bswap16(uint16_t x)
{
    return (x >> 8) | (x << 8);
}

inline uint32_t bswap32(uint32_t x)
{
    return (x >> 24) | ((x >> 8) & 0xff00) | ((x & 0xff00) << 8) | (x << 24);
}

inline uint64_t bswap64(uint64_t x)
{
    return (x >> 56) | ((x >> 40) & 0xff00) | ((x >> 24) & 0xff0000) | ((x >> 8) & 0xff000000) |
           ((x & 0xff000000) << 8) | ((x & 0xff0000) << 24) | ((x & 0xff00) << 40) | (x << 56);
}